#include "PPU.h"

#include <algorithm> //std::fill
#include <iterator> //std::size

#include <lodepng.h>
#include <nlohmann/json.hpp>

#include "Config.h"
#include "Gameboy.h"
#include "Model.h"
#include "util/Common.h"
#include "util/Log.h"
#include "util/StringUtils.h"

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
	bus(gb.bus),
	debug(gb.debug),
	interrupt(gb.interrupt),
	
	vram_backing(Memory::PAGE_SIZE * 4),
	vram_tag_backing(Memory::PAGE_SIZE * 2) {
	clean();
}

void PPU::install(Memory& bus, Model::Type model) {
	vram = vram_backing.map();
	vram_bank = vram_backing.map(bus.Mem() + 0x8000, 0x0000, Memory::PAGE_SIZE * 2);
	
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
		cgbMode = true;
		
		bus.register_io(0x4F, io);

		for (u8 i = 0x51; i < 0x56; ++i) bus.register_io(i, io);
		for (u8 i = 0x68; i < 0x6C; ++i) bus.register_io(i, io);
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
	BGP = OBP0 = OBP1 = { 0, 0, 0, 0 };
	WX = WY = 0;

	// Internal
	for (auto& displayBuf : buffers) {
		displayBuf.pixels.fill(DefaultPixel); // white
	}
	redraw = true;

	renderSprites.fill(0);

	cycles = 0;
	frameCycles = 0;
	lastTile = 0;
	last_stat = false;

	spritesScanned = 0;
	loadedSprites = 0;

	windowYTrigger = false;
	windowLines = 0;

	vblankHelper = false;

	framesPresented = 0;
}

// Called every CPU m-cycle
void PPU::update() {
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

	++cycles;
	++frameCycles;

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
		case 0x47: return BGP.read();
		case 0x48: return OBP0.read();
		case 0x49: return OBP1.read();
		case 0x4A: return WY;
		case 0x4B: return WX;

		// CGB Registers
		case 0x4F: return VBK;

		case 0x51: case 0x52: case 0x53: case 0x54: return 0xFF;
		case 0x55: return !activeVDMA << 7; 
		
		case 0x68: return (BGPI.autoInc << 6) | BGPI.addr;
		case 0x69: return bgColors[BGPI.addr];
		case 0x6A: return (OBPI.autoInc << 6) | OBPI.addr;
		case 0x6B: return objColors[OBPI.addr];

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

			vram_bank = {};
			vram_bank = vram_backing.map(bus.Mem() + 0x8000, Memory::PAGE_SIZE * 2 * (value & 0x1), Memory::PAGE_SIZE * 2);
			break;

		case 0x51: 
			srcVDMA &= 0xFF;
			srcVDMA |= value << 8;
			break;

		case 0x52: 
			srcVDMA &= 0xFF00;
			srcVDMA |= value & 0xF0;
			break;
			
		case 0x53: 
			dstVDMA &= 0xFF;
			dstVDMA |= value << 8;
			break;
			
		case 0x54:
			dstVDMA &= 0xFF00;
			dstVDMA |= value & 0xF0;
			break;

		case 0x55: 
			modeVDMA = value & 0x80;
			curVDMA = ((value & 0x7F) + 1) * 0x10;
			activeVDMA = true;
			break; 
		
		case 0x68: 
			BGPI.autoInc = value >> 6;
			BGPI.addr = value & 0x3F;
			break;

		case 0x69: 
			bgColors[BGPI.addr] = value;
			if (BGPI.autoInc) BGPI.addr++;
			break;

		case 0x6A:
			OBPI.autoInc = value >> 6;
			OBPI.addr = value & 0x3F;
			break;

		case 0x6B: 
			objColors[OBPI.addr] = value;
			if (OBPI.autoInc) OBPI.addr++;
			break;

		default: break;
	}
}

void PPU::writeIntercept(addr dst, u32 src, u8 data) {
	//LB_INFO(CG, "{:04X} = {:04X} {{ {:02X} }}", dst, src, data);

	u16 vramOffset = dst & 0x1FFF;

	vram_src_locations[vramOffset] = src;

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
	if (true) {
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

u16 PPU::_fetchTileAddr(bool method8000, u8 tileoffset) {
	u16 addr = 0;

	if (tileoffset & 0x80) {
		addr = 0x800;
	}
	else if (!method8000) {
		addr = 0x1000;
	}

	return addr + ((tileoffset & 0x7F) * 16);
}

std::array<u8, 2> PPU::_fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset) {
	size_t loc = _fetchTileAddr(method8000, tileoffset) + (yoffset * 2);
	return { vram[loc], vram[loc + 1] };
}

void PPU::scanline() {
	if (cycles < 63) {
		return;
	}
	
	std::array<u8, 2> rawLine = { 0, 0 };

	std::array<Pixel, 16> line = {};
	const auto map0 =  0x1800;
	const auto map1 =  0x1C00;
	constexpr u8 tileMaxX = (256 / T);

	for (u8 tileX = 0; tileX < (160 / T); ++tileX) {
		if (LCDC.displayPriority) {
			// Background
			u8 x = ((tileX * 8) + SCX) & 0xFF;
			u8 y = (LY + SCY) & 0xFF;

			auto map = &vram[(LCDC.bgMap) ? map1 : map0];
			u16 mapOffset = (x / T) + ((y / T) * tileMaxX);
			u8 tileOffset = map[mapOffset];
			u8 yOffset = y % T;
			u8 xShift = SCX % T;

			addr tileAddr = _fetchTileAddr(LCDC.tileSet, tileOffset);
			addr lineAddr = tileAddr + yOffset * 2;
			rawLine = { vram[lineAddr], vram[lineAddr + 1] };
			rawLine[0] <<= xShift;
			rawLine[1] <<= xShift;
			
			for (u8 lineX = xShift; lineX < T; ++lineX) {
				auto& pixel = line[lineX - xShift];
				pixel.src = (tileOffset & 0x80) ? tileOffset : (LCDC.tileSet << 8) | tileOffset;
				pixel.tile = vram_src_locations[tileAddr];
				pixel.x = lineX;
				pixel.y = yOffset;
			}

			if (xShift != 0) {
				if (x >= 248) {
					mapOffset -= 31;
				}
				else {
					mapOffset += 1;
				}

				tileOffset = map[mapOffset];
				tileAddr = _fetchTileAddr(LCDC.tileSet, tileOffset);
				lineAddr = tileAddr + yOffset * 2;
				rawLine[0] |= (vram[lineAddr] >> (8 - xShift));
				rawLine[1] |= (vram[lineAddr + 1] >> (8 - xShift));

				for (u8 lineX = 0; lineX < xShift; ++lineX) {
					auto& pixel = line[(8 - xShift) + lineX];
					pixel.src = (tileOffset & 0x80) ? tileOffset : (LCDC.tileSet << 8) | tileOffset;
					pixel.tile = vram_src_locations[tileAddr];
					pixel.x = lineX;
					pixel.y = yOffset;
				}
			}

			// Window
			if (windowEnabled && LCDC.windowDisplay) {
				s16 adjustedX = (WX - 7) / 8;
				yOffset = (windowLines % 8);

				if (tileX >= adjustedX && LY >= WY) {
					auto map = vram.get() + ((LCDC.windowMap) ? map1 : map0);
					mapOffset = (tileX - adjustedX) + ((windowLines / 8) * tileMaxX);
					tileOffset = map[mapOffset];

					tileAddr = _fetchTileAddr(LCDC.tileSet, tileOffset);
					lineAddr = tileAddr + yOffset * 2;
					rawLine = { vram[lineAddr], vram[lineAddr + 1] };
					windowYTrigger = true;
					
					for (u8 lineX = 0; lineX < T; ++lineX) {
						auto& pixel = line[lineX];
						pixel.src = (tileOffset & 0x80) ? tileOffset : (LCDC.tileSet << 8) | tileOffset;
						pixel.tile = vram_src_locations[tileAddr];
						pixel.x = lineX;
						pixel.y = yOffset;
					}
				}
			}
		}

		// nullptr = bgp
		std::array<PaletteData*, 8> palettes = {};

		if (spritesEnabled && LCDC.objDisplay) {
			std::array<u8, 8> bottomSpriteLine;
			bottomSpriteLine.fill(u8(-1));

			for (u8 i = 0; i < loadedSprites; ++i) {
				u8 spriteIndex = renderSprites[i];
				Sprite& sprite = sprites[spriteIndex];

 				u8 x = tileX * 8;
				if (!inRange(sprite.xPos - 8, x, x + 7) && !inRange(sprite.xPos - 1, x, x + 7)) {
					continue;
				}

				u8 y = (LY + 16 - sprite.yPos) % 16;
				if (sprite.yFlip()) {
					y = ((LCDC.objSize) ? 15 : 7) - y;
				}

				u8 tileOffset = sprite.tile;
				if (LCDC.objSize) { // 8x16
					tileOffset = (y < 8) ? (sprite.tile & 0xFE) : (sprite.tile | 0x1);
				}

				addr tileAddr = _fetchTileAddr(true, tileOffset);
				addr lineAddr = tileAddr + (y % 8) * 2;
				std::array<u8, 2> spriteLine = { vram[lineAddr], vram[lineAddr + 1] };

				u8 spX, itX, endX;
				if (x <= (sprite.xPos - 8)) {
					spX = 0;
					itX = sprite.xPos % 8;
					endX = 8;
				}
				else {
					itX = 0;
					endX = sprite.xPos % 8;
					spX = 8 - endX;
				}

				for (; itX < endX; ++itX, ++spX) {
					u8 bitX = spX;
					if (sprite.xFlip()) {
						bitX = 7 - bitX;
					}

					u8 s_c0 = getBit(spriteLine[1], 7 - bitX);
					u8 s_c1 = getBit(spriteLine[0], 7 - bitX);

					if (sprite.behindBG()) {
						// check if bg/window color is not 0
						if (getBit(rawLine[1], 7 - itX) || getBit(rawLine[0], 7 - itX)) {
							continue;
						}
					}

					if ((bottomSpriteLine[itX] > sprite.xPos) && (s_c0 || s_c1)) {
						palettes[itX] = &((sprite.useOBP1()) ? OBP1 : OBP0);
						bottomSpriteLine[itX] = sprite.xPos;
						setBit(rawLine[1], 7 - itX, s_c0);
						setBit(rawLine[0], 7 - itX, s_c1);

						auto& pixel = line[itX];
						pixel.src = tileOffset;
						pixel.tile = vram_src_locations[tileAddr];
						pixel.x = bitX;
						pixel.y = (y % 8);
						pixel.obj = true;
					}
				}
			}
		}


		for (u8 x = 0; x < 8; ++x) {
			u8 color = (getBit(rawLine[1], 7 - x) << 1) | getBit(rawLine[0], 7 - x);
			PaletteData& palette = (palettes[x]) ? *palettes[x] : BGP;

			size_t offset = (LY * 160) + (tileX * 8) + x;
			auto& pixel = buffers[backIdx].pixels[offset];
			// custom graphics shenanigans
			pixel = line[x];
			pixel.palette = palette;
			pixel.color = palette[color];
		}
	}

	setMode(HBlank);
}

void PPU::oamScan() {
	//scan 2 sprites for every cycle
	while (spritesScanned < (cycles * 2) && spritesScanned != 40 && loadedSprites != 10) {
		Sprite& sprite = sprites[spritesScanned];

		if ((LY + 16) >= sprite.yPos &&
			(LY + 16) < (sprite.yPos + ((LCDC.objSize) ? 16 : 8))) {

			renderSprites[loadedSprites++] = spritesScanned;
		}

		++spritesScanned;
	}

	if (cycles == 20) {
		setMode(Drawing);
		spritesScanned = 0;
	}
}

void PPU::hblank() {
	if (_nextLine()) {
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

		interrupt.request.vblank = true;
		backIdx = spare.exchange({backIdx, true}, std::memory_order_acq_rel).idx;

		++framesPresented;
	}

	if (_nextLine() && LY == 154) {
		// reset after the 10 "lines" of vblank
		frameCycles = 0;
		windowLines = 0;
		LY = 0;
		vblankHelper = false;

		setMode(Searching);
	}
}

// name is a bit ambigious but checks if we're going to the next line with the amount of cycles
bool PPU::_nextLine() {
	if (cycles >= 114) {
		cycles -= 114;

		LY++;
		return true;
	}

	return false;
}

void PPU::render(std::function<void (Framebuffer&)> callback) {
	bool update = spare.load(std::memory_order_relaxed).update;
	if (update) {
		frontIdx = spare.exchange({frontIdx, false}, std::memory_order_acq_rel).idx;
		redraw = true;
	}
	
	if (redraw) {
		redraw = false;
		callback(buffers[frontIdx]);
	}
}

static PaletteData basic = {0, 1, 2, 3};
template<size_t N>
void rowHelper(std::array<u8, N>& outData, size_t index, u8 bottom, u8 top, bool color0Invis = false, PaletteData& pallete = basic) {
	for (int x = 0; x < 8; ++x) {
		u8 bit = 7 - x;
		u8 color = (getBit(top, bit) << 1) | getBit(bottom, bit);

		auto& pixel = (color0Invis && color == pallete.color0) ? PPU::invisPixel : PPU::paletteColors[pallete[color]];
		std::copy_n((u8*)&pixel, 4, outData.begin() + ((index + x) * 4));
	}
}
/*
void PPU::dumpTiles(u8* outData, const size_t outW, const size_t tW, const size_t tH, const size_t baseOffset) {
	size_t tileOffset = baseOffset;

	for (size_t tX = 0, tY = 0; (tX * tY) < (tW * tH); ++tX) {
		if (tX == tW) {
			tX = 0;
			++tY;
			tileOffset += 0x100;
		}

		size_t pixelOffset = (tY * PPU::T);
		for (int y = 0; y < PPU::T; ++y) {
			u8 bottom = VRAM[tileOffset];
			u8 top = VRAM[tileOffset + 1];
			tileOffset += 2;

			for (int x = 0; x < T; ++x) {
				u8 bit = T - x - 1;
				u8 color = (getBit(top, bit) << 1) | getBit(bottom, bit);

				auto& pixel = PPU::paletteColors[pallete[color]];

				outData[pixelOffset + 0] = pixel.
				std::copy_n((u8*)&pixel, 4, outData.begin() + ((index + x) * 4));
			}
		}
	}
}*/

void PPU::dumpTile(u8* out, const size_t outW, const u16 tile) {
	if (tile > 0x17F) return;

	for (int y = 0; y < PPU::T; ++y) {
		auto line = vram.get() + (tile * 0x10) + (y * 2);

		for (int x = 0; x < T; ++x) {
			u8 bit = T - x - 1;
			u8 color = (getBit(line[1], bit) << 1) | getBit(line[0], bit);

			auto& pixel = PPU::paletteColors[color];
			*(out++) = pixel.r;
			*(out++) = pixel.g;
			*(out++) = pixel.b;
			(void)*(out++);// = pixel.a;
		}

		out += outW;
	}
}

//maybe add support for an auto option?
void PPU::dumpBGMap(std::array<u8, 256 * 256 * 4>& outData, bool bgMap, bool tileSet) {
	auto map = vram.get() + ((bgMap) ? 0x1C00 : 0x1800);

	for (int t = 0; t < 0x400; ++t) {
		for (int y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(tileSet, y, map[t]);

			size_t rgbIndex = ((t / 32) * 256 * 8) + ((t % 32) * 8) + (y * 256);
			rowHelper(outData, rgbIndex, tile[0], tile[1]);
		}
	}
}

void PPU::dumpTileMap(std::array<u8, 128 * 64 * 3 * 4>& outData) {
	for (int t = 0; t < 0x180; ++t) {
		for (int i = 0; i < 8; ++i) {
			u8 top = vram[(t * 16ll) + (i * 2ll)];
			u8 bottom = vram[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = ((t / 16) * 128 * 8) + ((t % 16) * 8) + (i * 128);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}

void PPU::dumpBGMapTiles(std::array<u8, 32 * 8 * 32 * 8 * 4>& outData, Pos2 min, Pos2 max, bool bgMap, bool tileSet) {
	auto map = vram.get() + ((bgMap) ? 0x1C00 : 0x1800);

	for (int ty = min.y; ty != max.y + 1; ++ty) {
		for (int tx = min.x; tx != max.x + 1; ++tx) {
			int t = tx + (ty * 32);
			for (int y = 0; y < 8; ++y) {
				auto tile = _fetchTileLine(tileSet, y, map[t]);

				size_t rgbIndex = (ty - min.y) * (32 * 8 * 8) + (tx - min.x) * 8 + y * (32 * 8);
				rowHelper(outData, rgbIndex, tile[0], tile[1]);
			}
		}
	}
}

void PPU::dumpTiles(std::array<u8, 32 * 8 * 32 * 8 * 4>& outData, u32 x1, u32 y1, u32 x2, u32 y2, u32 w, u32 h) {
	u32 tileOffset = 0;
	u32 rowOffset = 0;

	int tileEnd = x2 + 1 + (y2 * 16);
	for (int t = x1 + (y1 * 16); t < tileEnd; ++t) {
		size_t rgbTileIndex = (8 * tileOffset) + (32 * 8 * 8 * rowOffset);
		for (int i = 0; i < 8; ++i) {
			u8 top = vram[(t * 16ll) + (i * 2ll)];
			u8 bottom = vram[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = rgbTileIndex + (32 * 8 * i);
			rowHelper(outData, rgbIndex, top, bottom);
		}

		if (++tileOffset == w) {
			tileOffset = 0;
			if (++rowOffset == h) {
				return;
			}
		}
	}
}

void PPU::dumpSprites(std::array<u8, 64 * 40 * 4>& outData) {
	for (u8 obj = 0; obj < 40; ++obj) {
		for (u8 y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(true, y, sprites[obj].tile);

			size_t rgbIndex = ((obj / 8) * 64 * 8) + ((obj % 8) * 8) + (y * 64);
			rowHelper(outData, rgbIndex, tile[0], tile[1], true, (sprites[obj].useOBP1()) ? OBP1 : OBP0);
		}
	}
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
				std::erase(loc, ':');
				u32 a = stringToHex(loc);

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
