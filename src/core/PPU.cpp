#include "PPU.h"

#include <algorithm> //std::fill
#include <iterator> //std::size

#include "Config.h"
#include "Interrupt.h"
#include "SpriteManager.h"
#include "util/Common.h"
#include "util/Log.h"

PPU::PPU(Interrupt& interrupt, SpriteManager& spriteManager) :
	interrupt(interrupt), 
	spriteManager(spriteManager),
	currentBuffer(&buffers[0]),
	nextBuffer(&buffers[1]) {
	clean();
}

void PPU::clean() {
	// prevent DisplayWindow::render from using framebuffer when clearing
	std::unique_lock lock(vblank_m);

	// GB Registers
	VRAM.fill(0);
	sprites.fill({});

	LCDC.displayPriority = 0;
	LCDC.objDisplay = 0;
	LCDC.objSize = 0;
	LCDC.bgMap = 0;
	LCDC.tileSet = 0;
	LCDC.windowDisplay = 0;
	LCDC.windowMap = 0;
	LCDC.lcdDisplay = 0;

	STAT.mode = Mode::Searching;
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
		displayBuf.hashes.resize(((160 / 8) + 2) * ((144 / 8) + 2)); // reserve enough space for a full screen of multiple tiles
		displayBuf.pixels.fill({ 0b00, INVALID_ID, 0, 0 }); // white
	}

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
	if (LCDC.lcdDisplay == 0) {
		return;
	}

	// hack to return the mode back to searching from vblank
	if (cycles == 0 && LY == 0) {
		STAT.mode = Mode::Searching;
		vblankHelper = false;
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

		default: {
			throw std::runtime_error("Unknown mode");
		} break;
	}

	STAT.coincidence = LY == LYC;
	if (STAT.coincidence && STAT.lycInterrupt) {
		stat_state = true;
	}

	if (!last_stat && stat_state) {
		interrupt.requestLcdStat = true;
	}

	last_stat = stat_state;
}

const PPU::Framebuffer& PPU::getBuffer() {
	return *currentBuffer;
}

u8 PPU::read(u8 reg) {
	switch (reg) {
		case 0x40: //LCDC
			return (LCDC.lcdDisplay << 7) | (LCDC.windowMap << 6) | (LCDC.windowDisplay << 5) | (LCDC.tileSet << 4) |
				(LCDC.bgMap << 3) | (LCDC.objSize << 2) | (LCDC.objDisplay << 1) | (LCDC.displayPriority);

		case 0x41: //LCDC Status
			return 0x80 | (STAT.lycInterrupt << 6) | (STAT.oamInterrupt << 5) | (STAT.vblankInterrupt << 4) |
				(STAT.hblankInterrupt << 3) | (STAT.coincidence << 2) | (STAT.mode);

		case 0x42: return SCY;
		case 0x43: return SCX;
		case 0x44: return LY;
		case 0x45: return LYC;
		case 0x47: return BGP.read();
		case 0x48: return OBP0.read();
		case 0x49: return OBP1.read();
		case 0x4A: return WY;
		case 0x4B: return WX;

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
				STAT.mode = 0;
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
		case 0x47: BGP = value; break;
		case 0x48: OBP0 = value; break;
		case 0x49: OBP1 = value; break;
		case 0x4A: WY = value; break;
		case 0x4B: WX = value; break;

		default:
			//log
			break;
	}
}

#define VRAM_BLOCKING

u8 PPU::readVRAM(u16 offset) {
#ifdef VRAM_BLOCKING
	if (STAT.mode == PPU::Drawing) {
		return 0xFF;
	}
#endif // VRAM_BLOCKING

	return VRAM[offset];
}

void PPU::writeVRAM(u16 offset, u8 value) {
#ifdef VRAM_BLOCKING
	if (STAT.mode == PPU::Drawing) {
		return;
	}
#endif // VRAM_BLOCKING

	VRAM[offset] = value;

	spriteManager.dump(offset >> 4);

	//fprintf(log, "Write to VRAM $%04X: $%02X\n", loc, value); //PSI's Logger
}

u8 PPU::readOAM(u8 offset) {
#ifdef VRAM_BLOCKING
	// only allow reads during hblank and vblank 
	if (STAT.mode > 1) {
		return 0xFF;
	}
#endif // VRAM_BLOCKING

	return sprites[offset >> 2].read(offset & 0x3);
}

void PPU::writeOAM(u8 offset, u8 value, bool force) {
#ifdef VRAM_BLOCKING
	// only allow writes during hblank and vblank 
	if (STAT.mode > 1 && !force) { 
		return;
	}
#endif // VRAM_BLOCKING
	
	sprites[offset >> 2].write(offset & 0x3, value);
}

template<size_t N>
void rowHelper(std::array<u32, N>& outData, size_t index, u8 bottom, u8 top, bool color0Invis = false, PaletteData& pallete = PaletteData(0, 1, 2, 3)) {
	for (int x = 0; x < 8; ++x) {
		u8 bit = 7 - x;
		u8 color = (getBit(top, bit) << 1) | getBit(bottom, bit);

		if (color0Invis && color == pallete.color0) {
			outData[index + x] = PPU::invisPixel;
		}
		else {
			outData[index + x] = PPU::paletteColors[pallete[color]];
		}
	}
}

void PPU::scanline() {
	if (cycles < 63) {
		return;
	}
	
	std::array<u8, 2> rawLine = { 0, 0 };

	std::array<Pixel, 8> line = {};
	const auto map0 = VRAM.begin() + 0x1800;
	const auto map1 = VRAM.begin() + 0x1C00;
	const u8 tileMaxX = (256 / 8);

	for (u8 tileX = 0; tileX < (160 / 8); ++tileX) {
		// nullptr = bgp
		std::array<PaletteData*, 8> palettes = {};

		if (LCDC.displayPriority) {
			// Background
			u8 x = ((tileX * 8) + SCX) & 0xFF;
			u8 y = (LY + SCY) & 0xFF;

			auto map = (LCDC.bgMap) ? map1 : map0;
			u16 tileOffset = (x / 8) + ((y / 8) * tileMaxX);
			u8 yOffset = y % 8;
			u8 xShift = SCX % 8;

			rawLine = _fetchTileLine(LCDC.tileSet, yOffset, map[tileOffset]);
			rawLine[0] <<= xShift;
			rawLine[1] <<= xShift;
			
			size_t hash = spriteManager.getTileHash(_fetchTileAddr(LCDC.tileSet, map[tileOffset]));

			for (u8 lineX = 0; lineX < 8; ++lineX) {
				auto& pixel = line[lineX];
				pixel.hash = hash;
				pixel.x = lineX;
				pixel.y = yOffset;
			}
			
			if (xShift != 0) {
				if (x >= 248) {
					tileOffset -= 31;
				}
				else {
					tileOffset += 1;
				}

				auto nextLine = _fetchTileLine(LCDC.tileSet, yOffset, map[tileOffset]);
				rawLine[0] |= (nextLine[0] >> (8 - xShift));
				rawLine[1] |= (nextLine[1] >> (8 - xShift));
			}


			// Window
			if (windowEnabled && LCDC.windowDisplay) {
				s16 adjustedX = (WX - 7) / 8;
				if (tileX >= adjustedX && LY >= WY) {
					auto map = (LCDC.windowMap) ? map1 : map0;
					u16 tileOffset = (tileX - adjustedX) + ((windowLines / 8) * tileMaxX);

					rawLine = _fetchTileLine(LCDC.tileSet, windowLines % 8, map[tileOffset]);

					windowYTrigger = true;
				}
			}
		}

		if (spritesEnabled && LCDC.objDisplay) {
			std::array<u8, 8> bottomSpriteLine;
			bottomSpriteLine.fill(u8(-1));

			for (u8 i = 0; i < loadedSprites; ++i) {
				Sprite sprite = sprites[renderSprites[i]];

 				u8 x = tileX * 8;
				if (!inRange(sprite.xPos - 8, x, x + 7) && !inRange(sprite.xPos - 1, x, x + 7)) {
					continue;
				}

				u8 y = (LY + 16 - sprite.yPos) % 16;
				if (sprite.yFlip) {
					y = ((LCDC.objSize) ? 15 : 7) - y;
				}

				u8 tileOffset = 0;
				if (LCDC.objSize) { // 8x16
					tileOffset = (y < 8) ? (sprite.tile & 0xFE) : (sprite.tile | 0x1);
				}
				else { // 8x8
					tileOffset = sprite.tile;
				}
				std::array<u8, 2> spriteLine = _fetchTileLine(true, y % 8, tileOffset);

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
					if (sprite.xFlip) {
						bitX = 7 - bitX;
					}

					u8 s_c0 = getBit(spriteLine[1], 7 - bitX);
					u8 s_c1 = getBit(spriteLine[0], 7 - bitX);

					if (sprite.behindBG) {
						// check if bg/window color is not 0
						if (getBit(rawLine[1], 7 - itX) || getBit(rawLine[0], 7 - itX)) {
							continue;
						}
					}

					if ((bottomSpriteLine[itX] > sprite.xPos) && (s_c0 || s_c1)) {
						palettes[itX] = &((sprite.useOBP1) ? OBP1 : OBP0);
						bottomSpriteLine[itX] = sprite.xPos;
						setBit(rawLine[1], 7 - itX, s_c0);
						setBit(rawLine[0], 7 - itX, s_c1);

						// invalidate the pixel till we support sprites
						line[itX].hash = PPU::INVALID_ID;
					}
				}
			}
		}


		for (u8 x = 0; x < 8; ++x) {
			u8 color = (getBit(rawLine[1], 7 - x) << 1) | getBit(rawLine[0], 7 - x);

			auto& pixel = nextBuffer->pixels[(LY * 160) + (tileX * 8) + x]; 
			pixel = line[x];
			pixel.data = (palettes[x]) ? (*palettes[x])[color] : BGP[color];
		}
	}

	/*
	std::array<u8, 2> line = { 0, 0 };
	std::array<u8, 2> currentSprite = { 0, 0 };

	const auto map0 = VRAM.begin() + 0x1800;
	const auto map1 = VRAM.begin() + 0x1C00;

	for (u8 x = 0; x < 160; ++x) {
		PaletteData* palette = &BGP;
		u8 tileX = x % 8;

		if (tileX == 0) {
			//background
			if (LCDC.displayPriority) {
				u16 adjustedX = (x + SCX) & 0xFF;
				u16 y = (LY + SCY) & 0xFF;

				auto map = (LCDC.bgMap) ? map1 : map0;
				u8 xShift = SCX % 8;
				u16 offset = (adjustedX / 8) + ((y / 8) * 32);
				u8 yOffset = y % 8;
				if (xShift == 0) {
					line = _fetchTileLine(LCDC.tileSet, yOffset, map[offset]);
				}
				else {
					auto tlineL = _fetchTileLine(LCDC.tileSet, yOffset, map[offset]);

					offset = (adjustedX >= 248) ? offset - 31 : offset + 1;
					auto tlineR = _fetchTileLine(LCDC.tileSet, yOffset, map[offset]);

					line[0] = (tlineL[0] << xShift) | (tlineR[0] >> (8 - xShift));
					line[1] = (tlineL[1] << xShift) | (tlineR[1] >> (8 - xShift));
				}
			}

			//window
			if (LCDC.displayPriority && LCDC.windowDisplay) {
				s16 adjustedX = (WX - 7);
				if (x >= adjustedX) {
					if (LY >= WY) {
						auto map = (LCDC.windowMap) ? map1 : map0;
						u8 offset = map[((x - adjustedX) / 8) + ((windowLines / 8) * 32)];
						line = _fetchTileLine(LCDC.tileSet, windowLines % 8, offset);
						windowYTrigger = true;
					}
				}
			}
		}

		u8 c0 = getBit(line[1], 7 - tileX);
		u8 c1 = getBit(line[0], 7 - tileX);

		//sprites
		if (LCDC.objDisplay) {
			u8 bottomSpriteX = 255;

			for (u8 i = 0; i < loadedSprites; ++i) {
				Sprite sprite = sprites[renderSprites[i]];

				if (x < (sprite.xPos - 8) || x >= sprite.xPos) {
					continue;
				}

				u8 y = (LY + 16 - sprite.yPos) % 16;
				if (sprite.yFlip) {
					y = ((LCDC.objSize) ? 15 : 7) - y;
				}

				u8 tileY = y % 8;
				if (LCDC.objSize) { // 8x16
					currentSprite = _fetchTileLine(true, tileY, (y < 8) ? (sprite.tile & 0xFE) : (sprite.tile | 0x1));
				}
				else { // 8x8
					currentSprite = _fetchTileLine(true, tileY, sprite.tile);
				}

				u8 bitX = sprite.xPos - x - 1;
				if (sprite.xFlip) {
					bitX = 7 - bitX;
				}

				u8 s_c0 = getBit(currentSprite[1], bitX);
				u8 s_c1 = getBit(currentSprite[0], bitX);

				if (sprite.behindBG) {
					if (c0 != 0 || c1 != 0) {
						continue;
					}
				}

				if((bottomSpriteX > sprite.xPos) && (s_c0 || s_c1)) {
					palette = &((sprite.useOBP1) ? OBP1 : OBP0);
					bottomSpriteX = sprite.xPos;
					c0 = s_c0;
					c1 = s_c1;
				}
			}
		}

		(*nextBuffer)[x + (LY * 160)] = (*palette)[(c0 << 1) | c1];
	}
	*/

	STAT.mode = Mode::HBlank;
}

void PPU::oamScan() {
	//scan 2 sprites for every cycle
	while (spritesScanned < (cycles * 2) && spritesScanned != 40 && loadedSprites != 10) {
		Sprite sprite = sprites[spritesScanned];

		if (sprite.xPos != 0 &&
			(LY + 16) >= sprite.yPos &&
			(LY + 16) < (sprite.yPos + ((LCDC.objSize) ? 16 : 8))) {

			renderSprites[loadedSprites] = spritesScanned;
			++loadedSprites;
		}

		++spritesScanned;
	}

	if (cycles == 20) {
		STAT.mode = Mode::Drawing;
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

		if (LY == 144) {
			STAT.mode = Mode::VBlank; 
		}
		else {
			STAT.mode = Mode::Searching;
		}
	}
}

void PPU::vblank() {
	if (!vblankHelper) {
		vblankHelper = true;

		interrupt.requestVblank = true;

		std::unique_lock lock(vblank_m);

		std::swap(currentBuffer, nextBuffer);

		++framesPresented;
	}

	if (_nextLine() && LY == 154) {
		// reset after the 10 "lines" of vblank
		frameCycles = 0;
		windowLines = 0;
		LY = 0;
		STAT.mode = Mode::Searching;
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

	return std::array<u8, 2>{
		VRAM[loc],
		VRAM[loc + 1],
	};
}

//maybe add support for an auto option?
void PPU::dumpBGMap(std::array<u32, 256 * 256>& outData, bool bgMap, bool tileSet) {
	auto map = VRAM.begin() + ((bgMap) ? 0x1C00 : 0x1800);

	for (int t = 0; t < 0x400; ++t) {
		for (int y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(tileSet, y, map[t]);

			size_t rgbIndex = ((t / 32) * 256 * 8) + ((t % 32) * 8) + (y * 256);
			rowHelper(outData, rgbIndex, tile[0], tile[1]);
		}
	}
}

void PPU::dumpTileMap(std::array<u32, 128 * 64 * 3>& outData) {
	for (int t = 0; t < 0x180; ++t) {
		for (int i = 0; i < 8; ++i) {
			u8 top = VRAM[(t * 16ll) + (i * 2ll)];
			u8 bottom = VRAM[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = ((t / 16) * 128 * 8) + ((t % 16) * 8) + (i * 128);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}

void PPU::dumpSprites(std::array<u32, 64 * 40>& outData) {
	for (u8 obj = 0; obj < 40; ++obj) {
		for (u8 y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(true, y, sprites[obj].tile);

			size_t rgbIndex = ((obj / 8) * 64 * 8) + ((obj % 8) * 8) + (y * 64);
			rowHelper(outData, rgbIndex, tile[0], tile[1], true, (sprites[obj].useOBP1) ? OBP1 : OBP0);
		}
	}
}