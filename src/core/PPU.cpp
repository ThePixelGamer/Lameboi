#include "PPU.h"

#include <algorithm> //std::fill
#include <iterator> //std::size
#include <queue>

#include <lodepng.h>
#include <nlohmann/json.hpp>

#include "Config.h"
#include "Gameboy.h"
#include "Model.h"
#include "util/Common.h"
#include "util/Log.h"
#include "util/StringUtils.h"
#include "util/FIFO.h"

namespace fs = std::filesystem;

bool imageIndexColors(const Tile::Data& image) {
	for (auto& p : image) {
		if (std::count(indexColors.begin(), indexColors.end(), p) == 0) {
			return false;
		}
	}

	return true;
}


Pixel PPU::DefaultPixel;

PPU::PPU(Gameboy& gb) :
	inBios(gb.cpu.inBios),
	core(gb),
	bus(gb.bus),
	debug(gb.debug),
	interrupt(gb.interrupt),
	
	vram_backing(VRAM_SIZE * 2),
	vram_tag_backing(VRAM_SIZE) {
	clean();

	vram = vram_backing.map();
	model = Model::DMG;
}

void PPU::install(Memory& bus, Model::Type type) {
	model = type;

	vram_chunk.setup(bus.Mem() + 0x8000, 2);
	vram_chunk.banks = 2;

	vram_chunk.map(vram_backing, 0);
	
	vram_bus = bus.register_bus(nullptr, nullptr, nullptr);

	vram_tag = vram_tag_backing.map(bus.Tags() + 0x8000, 0, Memory::PAGE_SIZE * 2);
	std::ranges::fill(std::span(vram_tag.get<Memory::BusTag>(), Memory::PAGE_SIZE * 2), vram_bus);
	
	auto oam_read = [](void* d, addr a) -> u8 {
		return static_cast<PPU*>(d)->sprites[(a & 0xFF) >> 2][a & 0x3]; 
	};
	auto oam_write = [](void* d, addr a, u8 v) {
		static_cast<PPU*>(d)->sprites[(a & 0xFF) >> 2][a & 0x3] = v;
	};

	oam_bus = bus.register_bus(oam_read, oam_write, this);
	std::ranges::fill(std::span(bus.Tags() + 0xFE00, 0xA0), oam_bus);

	auto io = bus.register_bus(
		[](void* d, addr a) { return static_cast<PPU*>(d)->read(a & 0xFF); },
		[](void* d, addr a, u8 v) { static_cast<PPU*>(d)->write(a & 0xFF, v); },
		this
	);

	for (u8 i = 0x40; i < 0x4C; ++i) bus.register_io(i, io);

	if (model == Model::CGB) {
		dmgObjPriority = false;

		bus.register_io(0x4F, io);

		for (u8 i = 0x51; i < 0x56; ++i) bus.register_io(i, io);
		for (u8 i = 0x68; i < 0x6D; ++i) bus.register_io(i, io);
	}
}

void PPU::clean() {
	bios.load("lameboi");

	// GB Registers
	LCDC.displayPriority = 0;
	LCDC.objDisplay = 0;
	LCDC.objSize = 0;
	LCDC.bgMap = 0;
	LCDC.tileSet = 0;
	LCDC.windowDisplay = 0;
	LCDC.windowMap = 0;
	LCDC.lcdDisplay = 0;

	setMode(HBlank);
	STAT.coincidence = 0;
	STAT.hblankInterrupt = 0;
	STAT.vblankInterrupt = 0;
	STAT.oamInterrupt = 0;
	STAT.lycInterrupt = 0;

	SCX = SCY = 0;
	LY = 0;
	LYC = 0;
	BGP = OBP0 = OBP1 = 0;
	WX = WY = 0;

	// Internal
	for (auto& displayBuf : buffers) {
		displayBuf.pixels.fill(DefaultPixel); // white
	}
	redraw = true;

	renderSprites.fill(0);

	cycles = 0;
	lastTile = 0;
	last_stat = false;

	spritesScanned = 0;
	loadedSprites = 0;

	windowYTrigger = false;
	windowLines = 0;

	vblankHelper = false;

	framesPresented = 0;
	activeVDMA = false;

	srcVDMA = 0;
	dstVDMA = 0;
	curVDMA = 0;

	dmgObjPriority = true;
}

// Called every CPU m-cycle
void PPU::update(bool doubleSpeed) {
	// DMA Transfer, 1 byte per M Cycle
	if (dma != 0 && dma <= DMA_SIZE) {
		u8 idx = (DMA_SIZE - dma);
		u16 dmaAddr = (DMA_START << 8) | idx;
		bus.write(0xFE00 + idx, bus.read(dmaAddr));
		--dma;
	}

	if (LCDC.lcdDisplay == 0) {
		return;
	}

	cycles += (doubleSpeed) ? 2 : 4;

	bool stat_state = false;

	switch (STAT.mode) {
		case Mode::Searching:
			oamScan();

			if (STAT.oamInterrupt) {
				stat_state = true;
			}
			break;

		case Mode::Drawing:
			scanline();
			break;

		case Mode::HBlank:
			hblank();

			if (STAT.hblankInterrupt) {
				stat_state = true;
			}
			break;

		case Mode::VBlank:
			vblank();

			if (STAT.vblankInterrupt) {
				stat_state = true;
			}
			break;

		default: break;
	}

	STAT.coincidence = LY == LYC;
	if (STAT.coincidence && STAT.lycInterrupt) {
		stat_state = true;
	}

	if (!last_stat && stat_state) {
		interrupt.request.lcdStat = true;
	}

	last_stat = stat_state;
}

u8 PPU::read(u8 reg) {
	switch (reg) {
		case 0x40: //LCDC
			return (LCDC.lcdDisplay << 7) | (LCDC.windowMap << 6) | (LCDC.windowDisplay << 5) | (LCDC.tileSet << 4) |
				(LCDC.bgMap << 3) | (LCDC.objSize << 2) | (LCDC.objDisplay << 1) | (LCDC.displayPriority);

		case 0x41: //LCDC Status
			return 0x80 | 
				(STAT.lycInterrupt << 6) | 
				(STAT.oamInterrupt << 5) | 
				(STAT.vblankInterrupt << 4) |
				(STAT.hblankInterrupt << 3) | 
				(STAT.coincidence << 2) | 
				(LCDC.lcdDisplay) ? (STAT.mode) : 0;

		case 0x42: return SCY;
		case 0x43: return SCX;
		case 0x44: return LY;
		case 0x45: return LYC;
		case 0x46: return DMA_START;
		case 0x47: return BGP;
		case 0x48: return OBP0;
		case 0x49: return OBP1;
		case 0x4A: return WY;
		case 0x4B: return WX;

		// CGB Registers
		case 0x4F: return VBK;

		case 0x51: case 0x52: case 0x53: case 0x54: return 0xFF;
		case 0x55: return curVDMA - 1; 
		
		case 0x68: return (BGPI.autoInc << 6) | BGPI.addr;
		case 0x69: return bgColors[BGPI.addr >> 3].color[BGPI.addr & 0x7];
		case 0x6A: return (OBPI.autoInc << 6) | OBPI.addr;
		case 0x6B: return objColors[OBPI.addr >> 3].color[OBPI.addr & 0x7];

		case 0x6C: return u8(dmgObjPriority);

		default:
			//log
			return 0xFF;
	}
}

void PPU::write(u8 reg, u8 value) {
	switch (reg) {
		case 0x40: //LCDC
			LCDC.displayPriority = (value);
			LCDC.objDisplay = (value >> 1);
			LCDC.objSize = (value >> 2);
			LCDC.bgMap = (value >> 3);
			LCDC.tileSet = (value >> 4);
			LCDC.windowDisplay = (value >> 5);
			LCDC.windowMap = (value >> 6);
			LCDC.lcdDisplay = (value >> 7);
			
			if (LCDC.lcdDisplay == 0) {
				LY = 0;
				setMode(HBlank);
				cycles = 0;
			}
			break;

		case 0x41: //LCDC Status
			STAT.hblankInterrupt = (value >> 3);
			STAT.vblankInterrupt = (value >> 4);
			STAT.oamInterrupt = (value >> 5);
			STAT.lycInterrupt = (value >> 6);
			break;

		case 0x42: SCY = value;	break;
		case 0x43: SCX = value;	break;
		case 0x45: LYC = value;	break;
		case 0x46: 
			if (dma == 0) {
				DMA_START = value;
				dma = DMA_SIZE;
			}
			break;
		case 0x47: BGP = value; break;
		case 0x48: OBP0 = value; break;
		case 0x49: OBP1 = value; break;
		case 0x4A: WY = value; break;
		case 0x4B: WX = value; break;

		case 0x4F:
			VBK = value;
			vram_chunk.map(vram_backing, VBK & 0x1);
			break;

		case 0x51: 
			srcVDMA &= 0xF0;
			srcVDMA |= value << 8;
			break;

		case 0x52: 
			srcVDMA &= 0xFF00;
			srcVDMA |= value & 0xF0;
			break;
			
		case 0x53: 
			dstVDMA &= 0xF0;
			dstVDMA |= value << 8;
			break;
			
		case 0x54:
			dstVDMA &= 0xFF00;
			dstVDMA |= value & 0xF0;
			break;

		case 0x55:
			modeVDMA = (value & 0x80);
			curVDMA = (value & 0x7F) + 1;

			if (activeVDMA) {
				if (!modeVDMA) activeVDMA = false;
			}
			else {
				if (!modeVDMA) {
					while (curVDMA) {
						vdma();
					}
					--curVDMA;
				}
				else {
					activeVDMA = true;
					LB_INFO(PPU, "0x{:04X} -> 0x{:04X} ({})", srcVDMA, dstVDMA, curVDMA);
				}
			}

			break;
		
		case 0x68: 
			BGPI.autoInc = value >> 7;
			BGPI.addr = value & 0x3F;
			break;

		case 0x69: 
			bgColors[BGPI.addr >> 3].color[BGPI.addr & 0x7] = value;
			if (BGPI.autoInc) BGPI.addr++;
			break;

		case 0x6A:
			OBPI.autoInc = value >> 7;
			OBPI.addr = value & 0x3F;
			break;

		case 0x6B: 
			objColors[OBPI.addr >> 3].color[OBPI.addr & 0x7] = value;
			if (OBPI.autoInc) OBPI.addr++;
			break;

		case 0x6C: dmgObjPriority = value & 0x1; break;

		default: break;
	}
}

void PPU::writeIntercept(addr dst, u32 src, u8 data) {
	//LB_INFO(CG, "{:04X} = {:04X} {{ {:02X} }}", dst, src, data);

	u16 vramOffset = dst & 0x1FFF;

	vram_src_locations[vram_chunk.activeBank * VRAM_SIZE + vramOffset] = src;

	// 0x8000 - 0x97FF
	if (vramOffset < 0x1800) {
	}
	// 0x9800 - 0x9FFF
	else {
		// todo: use writes to check for matching logic
	}
}

void PPU::setMode(Mode mode) {
	STAT.mode = mode;

	// HBlank -> Searching turn off OAM
	// Searching -> Drawing turn off VRAM
	// Drawing -> HBlank turn on everything
	// HBlank -> VBlank do nothing
	if (false) {
		switch (STAT.mode) {
			case Drawing: 
				bus.buses[vram_bus.id].enable = false;
			case Searching:
				bus.buses[oam_bus.id].enable = false;
				break;
				
			case HBlank:
				bus.buses[vram_bus.id].enable = true;
				bus.buses[oam_bus.id].enable = true;
				break;

			default: break;
		}
	}
}

u16 _fetchTileAddr(bool method8000, u8 tileoffset, u8 bank = 0) {
	u16 addr = PPU::VRAM_SIZE * bank;

	if (tileoffset & 0x80) {
		addr += 0x800;
	}
	else if (!method8000) {
		addr += 0x1000;
	}

	return addr + ((tileoffset & 0x7F) * 16);
}

u16 _fetchTileIdx(bool method8000, u8 tileoffset, bool bank) {
	u16 idx = 384 * bank;

	if (tileoffset & 0x80) {
		idx += 128;
	}
	else if (!method8000) {
		idx += 256;
	}

	return idx + (tileoffset & 0x7F);
}

void PPU::scanline() {
	if (cycles < 252) {
		return;
	}

	// Standard render
	{
		struct Pixel {
			u8 col : 2 = 0;
			u8 pal : 3 = 0;
			u8 pr : 6 = 0;
			u8 bg_pr : 1 = 0;
		};

		FIFO<Pixel, 8> bg, obj;

		const auto map0 = 0x1800;
		const auto map1 = 0x1C00;
		u16 baseMap = (LCDC.bgMap) ? map1 : map0;

		constexpr u8 tileMaxX = (256 / T);
		u8 bg_x = (SCX / T) & 0x1F;
		u8 y = LY + SCY;
		u16 yOffset = (y / T) * tileMaxX;

		for (u16 p = 0; p < 160; ++p) {
			if (windowEnabled && LCDC.windowDisplay && (std::max(WX - 7, 0)) == p && LY >= WY) {
				bg.reset();
				bg_x = 0;
				baseMap = (LCDC.windowMap) ? map1 : map0;
				y = windowLines++;
				yOffset = (y / T) * tileMaxX;
			}

			if (bg.size() == 0) {
				u16 tile = baseMap + yOffset + bg_x;
				bg_x = (bg_x + 1) & 0x1F;
				u8 bgattr = (core.cgbMode) ? vram[VRAM_SIZE + tile] : (LCDC.displayPriority << 7);
				addr tileAddr = _fetchTileAddr(LCDC.tileSet, vram[tile], bool(bgattr & 0x8));

				u8 tileY = (bgattr & 0x40) ? 7 - y : y; 
				addr lineAddr = tileAddr + (tileY % T) * 2;
				
				u8 b_col = vram[lineAddr], t_col = vram[lineAddr + 1]; 
				for (u8 c = 0; c < 8; ++c) {
					u8 bitX = (bgattr & 0x20) ? 7 - c : c;
						
					bg.push({
						.col = u8((getBit(t_col, 7 - bitX) << 1) | getBit(b_col, 7 - bitX)),
						.pal = u8(bgattr & 0x7), 
						.bg_pr = bool(bgattr & 0x80)
					});
				}
			}

			if (p == 0) {
				for (u8 i = 0; i < SCX % T; ++i)
					bg.pop();
			}

			while (obj.size() < 8) {
				obj.push({
					.col = 0
				});
			}
			
			if (spritesEnabled && LCDC.objDisplay) {
				for (u8 i = 0; i < loadedSprites; ++i) {
					u8 spriteIndex = renderSprites[i];
					Sprite& sprite = sprites[spriteIndex];

					if (p != (sprite.xPos - 8)) {
						continue; 
					}

					u8 objY = (LY + 16 - sprite.yPos) % 16;
					if (sprite.yFlip()) {
						objY = ((LCDC.objSize) ? 15 : 7) - objY;
					}

					u8 tileOffset = sprite.tile;
					if (LCDC.objSize) { // 8x16
						tileOffset = (objY < 8) ? (sprite.tile & 0xFE) : (sprite.tile | 0x1);
					}

					addr tileAddr = _fetchTileAddr(true, tileOffset, (core.cgbMode) ? sprite.getBank() : 0);
					addr lineAddr = tileAddr + (objY % 8) * 2;

					u8 b_col = vram[lineAddr], t_col = vram[lineAddr + 1];
					u8 priority = (dmgObjPriority) ? 0 : spriteIndex;
					for (u8 c = 0; c < 8; ++c) {
						u8 bitX = (sprite.xFlip()) ? 7 - c : c;
						
						auto& pixel = obj[c];
						u8 newCol = (getBit(t_col, 7 - bitX) << 1) | getBit(b_col, 7 - bitX);
						if (newCol != 0 && (pixel.col == 0 || pixel.pr > priority)) {
							pixel.col = newCol;
							pixel.pal = (core.cgbMode) ? sprite.getCGBPal() : sprite.useOBP1();
							pixel.pr = priority;
							pixel.bg_pr = sprite.behindBG();
						}
					}
				}
			}

			size_t offset = (LY * 160) + p;
			auto& pixel = buffers[backIdx].pixels[offset];
			auto bgPixel = bg.pop();
			auto objPixel = obj.pop();

			auto objWin = [&](bool cgbMode) {
				if (objPixel.col != 0) {
					if (cgbMode) {
						if (!LCDC.displayPriority) {
							return true;
						}
					}

					if (!objPixel.bg_pr) {
						if (!cgbMode || !bgPixel.bg_pr) {
							return true;
						}
					}

					if (bgPixel.col == 0) {
						return true;
					}
				}

				return false;
			};

			if (objWin(core.cgbMode)) {
				if (model == Model::CGB) {
					pixel.color = objColors[objPixel.pal][objPixel.col];
				}
				else {
					auto& pal = (objPixel.pal) ? OBP1 : OBP0;
					pixel.color = pal[objPixel.col];
				}
			}
			else {
				if (model == Model::CGB) {
					pixel.color = (core.cgbMode || bgPixel.bg_pr) ? bgColors[bgPixel.pal][bgPixel.col] : 0x7FFF;
				}
				else {
					pixel.color = (bgPixel.bg_pr) ? BGP[bgPixel.col] : 0;
				}
			}
		}
	}
	
	// CG metainfo
	auto& meta = buffers[backIdx].meta;
	auto& line = meta.lines[LY];
	auto& bg = line.bg;
	bg.x = SCX;
	bg.y = SCY;
	bg.altMap = LCDC.bgMap;

	auto& window = line.window;
	window.x = WX;
	window.y = WY;
	window.enabled = LCDC.windowDisplay;
	window.altMap = LCDC.windowMap;

	line.altTileSet = LCDC.tileSet;

	setMode(HBlank);
}

void PPU::oamScan() {
	//scan 2 sprites for every cycle
	while (spritesScanned < (cycles / 2) && spritesScanned != 40 && loadedSprites != 10) {
		Sprite& sprite = sprites[spritesScanned];

		if ((LY + 16) >= sprite.yPos &&
			(LY + 16) < (sprite.yPos + ((LCDC.objSize) ? 16 : 8))) {

			renderSprites[loadedSprites++] = spritesScanned;
		}

		++spritesScanned;
	}

	if (cycles >= 80) {
		setMode(Drawing);
		spritesScanned = 0;
	}
}

void PPU::hblank() {
	if (_nextLine()) {
		if (activeVDMA && curVDMA) {
			vdma();
		}

		if (windowYTrigger) {
			windowYTrigger = false;
			if (++windowLines >= 144) {
				windowLines = 0;
			}
		}

		loadedSprites = 0;

		setMode((LY == 144) ? VBlank : Searching);
	}
}

void PPU::vblank() {
	if (!vblankHelper) {
		vblankHelper = true;
		debug.inVblank = true;

		auto& meta = buffers[backIdx].meta;
		for (int i = 0; i < meta.tiles.size(); ++i) {
			u16 addr = VRAM_SIZE * (i >= meta.tiles.size() / 2);
			addr += (i % 384) * 16;
			
			u64& hash = meta.tiles[i].hash;
			hash = 0;
			for (u8 i = 0; i < 8; ++i) {
				u16 offset = addr + (i * 2);
				hash |= u64(vram[offset] | vram[offset + 1]) << (i * 8);
			}

			meta.tiles[i].src = vram_src_locations[addr];
		}

		if (model == Model::CGB) {
			for (int i = 0; i < meta.palettes.size(); ++i) {
				meta.palettes[i].cgb = (i >= meta.palettes.size() / 2) ? objColors[i % 8] : bgColors[i % 8];
			}
		}
		else {
			meta.palettes[0].dmg = BGP;
			meta.palettes[8].dmg = OBP0;
			meta.palettes[9].dmg = OBP1;
		}

		for (int i = 0; i < meta.maps.size(); ++i) {
			u8 a = vram[VRAM_SIZE + 0x1800 + i];
			
			meta.maps[i] = {
				.idx = vram[0x1800 + i],
				.pal = u8(a & 0x7),
				.altBank = u8(a & 0x8),
				.xFlip = u8(a & 0x20),
				.yFlip = u8(a & 0x40),
				.priority = (core.cgbMode) ? u8(a & 0x80) : u8(LCDC.displayPriority)
			};
		}

		for (int i = 0; i < meta.sprites.size(); ++i) {
			auto& sprite = sprites[i];
			
			meta.sprites[i] = {
				.idx = sprite.tile,
				.x = sprite.xPos,
				.y = sprite.yPos,
				.pal = (core.cgbMode) ? sprite.getCGBPal() : u8(sprite.useOBP1()),
				.altBank = (core.cgbMode) ? u8(sprite.getBank()) : u8(0),
				.xFlip = sprite.xFlip(), .yFlip = sprite.yFlip(),
				.behindBG = sprite.behindBG(),
			};
		}

		interrupt.request.vblank = true;
		backIdx = spare.exchange({backIdx, true}, std::memory_order_acq_rel).idx;

		++framesPresented;
	}

	if (_nextLine() && LY == 154) {
		// reset after the 10 "lines" of vblank
		windowLines = 0;
		LY = 0;
		vblankHelper = false;

		setMode(Searching);
	}
}

void PPU::vdma() {
	--curVDMA;
	for (u8 i = 0; i < 0x10; ++i) {
		bus.write(0x8000 | dstVDMA++, bus.read(srcVDMA++));
		if (i & 0x1) core.step();
	}
}

// name is a bit ambigious but checks if we're going to the next line with the amount of cycles
bool PPU::_nextLine() {
	if (cycles >= 456) {
		cycles -= 456;

		LY++;
		return true;
	}

	return false;
}

Framebuffer* PPU::getNextBuffer() {
	bool update = spare.load(std::memory_order_relaxed).update;
	if (update) {
		frontIdx = spare.exchange({frontIdx, false}, std::memory_order_acq_rel).idx;
		redraw = true;
	}
	
	if (redraw) {
		redraw = false;
		return &buffers[frontIdx];
	}
	
	return nullptr;
}

std::array<u8, 16> PPU::dumpTile(const u16 tile, bool altBank) {
	assert(tile < 0x180);
	
	std::array<u8, 16> tileOut;
	size_t offset = (altBank * VRAM_SIZE) + (tile * 0x10);
	std::memcpy(tileOut.data(), &vram[offset], sizeof(tileOut));
	return tileOut;
}

/// Custom Graphics

void Manifest::load(const std::string& n) {
	name = n;
	folder = name + "/";
	if (!fs::exists(getPath())) {
		return;
	}

	json manifest;
	if (validate(manifest, getPath() + "manifest.json")) {
		if (manifest.contains("skins")) {
			auto& jsonSkins = manifest["skins"];

			if (jsonSkins.is_array()) {
				for (auto& skin : jsonSkins) {
					if (skin.is_string()) skins.push_back(skin);
				}
			}
		}

		if (manifest.contains("sprite")) {
			if (skins.empty()) {
				loadSprite("", getPath(), manifest["sprite"]);
			}
			else {
				for (auto& skin : skins) {
					if (selSkin.empty()) selSkin = skin;
					std::string path = getPath() + skin + "/";
					if (!fs::exists(path)) {
						path.pop_back();
					}
					loadSprite(skin, path, manifest["sprite"]);
				}
			}
		}

		if (manifest.contains("tilemap")) {
			if (skins.empty()) {
				loadTilemap("", getPath(), manifest["tilemap"]);
			}
			else {
				for (auto& skin : skins) {
					if (selSkin.empty()) selSkin = skin;
					std::string path = getPath() + skin + "/";
					if (!fs::exists(path)) {
						path.pop_back();
					}
					loadTilemap(skin, path, manifest["tilemap"]);
				}
			}
		}

		LB_INFO(CG, "Loaded {}", name);
	}
	else {
		LB_ERROR(CG, "Missing manifest for {}", folder);
	}
}

void Manifest::loadSprite(const std::string& skin, const std::string& path, const json& sprite) {
	if (sprite.is_object()) {
		for (auto& [name, sprites] : sprite.items())
			loadSprite(skin, path + name , sprites);
		return;
	}

	std::vector<Color> image;
	u32 width, height;

	unsigned error = lodepng::decode((std::vector<u8>&)image, width, height, path + ".png");

	// if there's an error, display it and skip
	if (error) {
		LB_ERROR(CG, "lodepng error {}: {} with {}.png", error, lodepng_error_text(error), path);
		return;
	}

	// verify sprite is 8x8 tile based
	if ((width % 8) != 0 || (height % 8) != 0) {
		LB_ERROR(CG, "Tilemap not in 8x8 format");
		return;
	}
	
	bool indexed = imageIndexColors(image);
	size_t tileWidth = width / 8;
	for (auto& cond_json : sprite) {
		if (cond_json.is_string()) {
			std::string cond = cond_json;
			size_t x = 0, y = 0;

			size_t eq = cond.find('=');
			if (eq != cond.npos) {
				// todo: validate
				size_t comma = cond.find(',');
				x = std::stoull(cond.substr(0, comma));
				y = std::stoull(cond.substr(comma + 1, eq - comma - 1));
				cond = cond.substr(eq + 1);
			}

			addr a = stringToHex(cond);
			
			LB_INFO(CG, "{},{}={}", x, y, a);
		}
	}
}

void Manifest::loadTilemap(const std::string& skin, const std::string& path, const json& tilemap) {
	if (tilemap.is_object()) {
		for (auto& [name, tiles] : tilemap.items())
			loadTilemap(skin, path + name , tiles);
		return;
	}

	std::vector<Color> image;
	u32 width, height;

	unsigned error = lodepng::decode((std::vector<u8>&)image, width, height, path + ".png");

	// if there's an error, display it and skip
	if (error) {
		LB_ERROR(CG, "lodepng error {}: {} with {}.png", error, lodepng_error_text(error), path);
		return;
	}

	// verify sprite is 8x8 tile based
	if ((width % 8) != 0 || (height % 8) != 0) {
		LB_ERROR(CG, "Tilemap not in 8x8 format");
		return;
	}

	bool indexed = imageIndexColors(image);
	size_t tileOffset = 0;
	size_t tileWidth = width / 8;
	for (auto& loc_json : tilemap) {
		auto load = [&](std::string loc) {
			if (!loc.empty()) {
				u32 a = 0;
				auto pos = loc.find(':');
				if (pos == loc.npos) {
					a = stringToHex(loc);
				}
				else {
					u32 b = stringToHex(loc.substr(0, pos));
					a = stringToHex(loc.substr(pos + 1));
					a += b << 14;
				}

				Tile::Data uvImage(8 * 8);

				size_t x = (tileOffset % tileWidth) * 8;
				size_t y = (tileOffset / tileWidth) * 8;
				for (size_t i = 0; i < 8; i++) {
					auto it = image.begin() + x + ((i + y) * width);
					std::copy_n(it, 8, uvImage.begin() + (i * 8));
				}

				auto& tile = tiles[a][skin];
				tile.set(std::move(uvImage), indexed);
			}
		};

		if (loc_json.is_string()) {
			load(loc_json);
		}
		else if (loc_json.is_array()) {
			for (auto& multiple : loc_json) {
				if (multiple.is_string()) {
					load(multiple);
				}
			}
		}

		++tileOffset;
	}
}

bool Manifest::validate(json& manifest, const std::string& manifestPath) {
	if (!fs::exists(manifestPath)) {
		return false;
	}

	std::fstream manifestFile(manifestPath);
	manifestFile >> manifest;

	if (!manifest.contains("tilemap")) {
		LB_ERROR(CG, "Manifest missing required objects.");
		return false;
	}

	return true;
}

/*
std::vector<Color> SpriteManager::getTilePixels(u16 tileOffset) {
	std::vector<Color> tile(8 * 8);
	
	for (int i = 0; i != 16; i += 2) {
		u8 bottom = ppu.vram[tileOffset + i];
		u8 top = ppu.vram[tileOffset + i + 1];

		// overwrite pixels vector
		for (u8 x = 0; x < 8; ++x) {
			u8 bit = 7 - x;
			u8 colorIdx = (getBit(top, bit) << 1) | getBit(bottom, bit);

			u8 y = (i / 2) * 8;
			tile[x + y] = indexColors[colorIdx];
		}
	}

	return tile;
}

void SpriteManager::dumpTile(addr offset) {
	auto& map = getManifest();
	addr tileSrc = ppu.vram_src_locations[offset];

	if (map.tiles.find(tileSrc) == map.tiles.end()) {
		std::vector<Color> tilePixels = getTilePixels(offset);

		std::stringstream ss;
		ss << std::hex << tileSrc;
		std::string tileSrcStr = ss.str();

		LB_INFO(PPU, "Dumping tile {} with src 0x{}", offset >> 4, tileSrcStr);

		// write vector to png
		unsigned error = lodepng::encode(map.getPath() + tileSrcStr + ".png", (std::vector<u8>&)tilePixels, 8, 8);

		// if there's an error, display it
		if (error)
			LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
	}
}*/
