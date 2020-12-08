#include "PPU.h"

#include <algorithm> //std::fill
#include <iterator> //std::size
#include <iostream>

#include "Util/Common.h"

PPU::PPU(Interrupt& interrupt) : interrupt(interrupt) {
	clean();
}

void PPU::clean() {
	//regs
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

	//internal
	display.fill(ColorPalette[0]); //white
	displayPresent.fill(ColorPalette[0]); //white
	renderSprites.fill(0);

	cycles = 0;
	lastTile = 0;

	spritesScanned = 0;
	loadedSprites = 0;

	windowYTrigger = false;
	windowLines = 0;

	vblankHelper = false;

	vblankCount = 0;
	isVblank = false;
}

//called every mcycle
void PPU::update() {
	if (LCDC.lcdDisplay == 0) {
		STAT.mode = Mode::Searching;
		LY = 0;
		cycles = 0;

		return;
	}

	// v-blank helper to return the mode back to searching
	if (cycles == 0 && LY == 0) {
		STAT.mode = Mode::Searching;
		vblankHelper = false;
	}

	++cycles;

	switch (STAT.mode) {
		case Mode::Searching:
			oamScan();
			break;

		case Mode::Drawing:
			scanline();
			break;

		case Mode::HBlank:
			hblank();
			break;

		case Mode::VBlank:
			vblank();
			break;

		default: {
			throw std::runtime_error("Unknown mode");
		} break;
	}

	STAT.coincidence = LY == LYC;
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

u8 PPU::readVRAM(u16 offset) {
	if (STAT.mode != PPU::Drawing) {
		return VRAM[offset];
	}

	return 0xFF;
}

void PPU::writeVRAM(u16 offset, u8 value) {
	if (STAT.mode != PPU::Drawing) {
		VRAM[offset] = value;

		//fprintf(log, "Write to VRAM $%04X: $%02X\n", loc, value); //PSI's Logger
	}
}

u8 PPU::readOAM(u8 offset) {
	if (STAT.mode < 2) {
		return sprites[offset >> 2].read(offset & 0x3);
	}

	return 0xFF;
}

void PPU::writeOAM(u8 offset, u8 value, bool force) {
	if (STAT.mode < 2 || force) { // only allow writes during hblank and vblank 
		sprites[offset >> 2].write(offset & 0x3, value);
	}
}

template<size_t N>
void rowHelper(std::array<u32, N>& outData, size_t index, u8 bottom, u8 top, bool color0Invis = false, Palette& pallete = Palette(0, 1, 2, 3)) {
	for (int x = 0; x < 8; ++x) {
		u8 bit = 7 - x;
		u8 color = (getBit(top, bit) << 1) | getBit(bottom, bit);

		if (color0Invis && color == pallete.color0) {
			outData[index + x] = PPU::invisPixel;
		}
		else {
			outData[index + x] = PPU::ColorPalette[pallete[color]];
		}
	}
}

template<size_t N>
void pixelPusher(std::array<u32, N>& outData, u8 x, u64 y, u8 pixel /* 00, 01, 10, 11 */, Palette& pallete) {
	outData[x + y] = PPU::ColorPalette[pallete[pixel]];
}

void PPU::fifo() {

}

void PPU::scanline() {
	if (cycles < 63) {
		return;
	}

	std::array<u8, 2> line = { 0, 0 };
	std::array<u8, 2> currentSprite = { 0, 0 };

	for (u8 x = 0; x < 160; ++x) {
		Palette* palette = &BGP;
		u8 tileX = x % 8;

		if (tileX == 0) {
			//background
			if (LCDC.displayPriority) {
				auto map = VRAM.begin() + ((LCDC.bgMap) ? 0x1C00 : 0x1800);

				u16 adjustedX = x + SCX;
				adjustedX = (adjustedX > 255) ? adjustedX - 256 : adjustedX;

				u16 y = LY + SCY;
				if (y > 255) {
					y -= 256;
				}

				if ((SCX % 8) == 0) {
					u8 offset = map[(adjustedX / 8ll) + ((y / 8ll) * 32)];
					line = _fetchTileLine(LCDC.tileSet, y % 8, offset);
				}
				else {
					u8 xShift = SCX % 8;
					u16 offset = (adjustedX / 8ll) + ((y / 8ll) * 32);
					auto tlineL = _fetchTileLine(LCDC.tileSet, y % 8, map[offset]);

					offset = (adjustedX >= 248) ? offset - 31 : offset + 1;
					auto tlineR = _fetchTileLine(LCDC.tileSet, y % 8, map[offset]);
					line = {
						u8((tlineL[0] << xShift) | (tlineR[0] >> (8 - xShift))),
						u8((tlineL[1] << xShift) | (tlineR[1] >> (8 - xShift))),
					};
				}
			}

			//window
			if (LCDC.displayPriority && LCDC.windowDisplay) {
				auto map = VRAM.begin() + ((LCDC.windowMap) ? 0x1C00 : 0x1800);

				s16 adjustedX = (WX - 7);
				if (x >= adjustedX) {
					if (LY >= WY) {
						u8 offset = map[((x - adjustedX) / 8) + ((windowLines / 8ll) * 32)];
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

				// yPos = 26, LY = 12, spriteSize = 8
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

		if (presenting) {
			pixelPusher(display, x, LY * 160ll, (c0 << 1) | c1, *palette);
		}
		else {
			pixelPusher(displayPresent, x, LY * 160ll, (c0 << 1) | c1, *palette);
		}
	}

	STAT.mode = Mode::HBlank;
}

void PPU::oamScan() {
	//scan 2 sprites for every cycle
	while (spritesScanned < (cycles * 2) && spritesScanned != 40 && loadedSprites != 10) {
		if (STAT.oamInterrupt) {
			interrupt.requestLcdStat = true;
		}

		Sprite sprite = sprites[spritesScanned];

		if (sprite.xPos != 0 &&
			(LY + 16) >= sprite.yPos &&
			(LY + 16) < (sprite.yPos + ((LCDC.objSize) ? 16 : 8))) {

			renderSprites[loadedSprites] = spritesScanned;
			++loadedSprites;
		}

		++spritesScanned;
	}

	if (cycles > 20) {
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

		if (STAT.hblankInterrupt) {
			interrupt.requestLcdStat = true;
		}

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
		if (STAT.vblankInterrupt) {
			interrupt.requestLcdStat = true;
		}

		std::unique_lock lock(vblank_m);

		presenting = !presenting;

		/*
		vblank.wait(lock, [this] { return isVblank; });
		isVblank = false;
		*/
	}

	if (_nextLine()) {
		if (LY >= 154) {
			windowLines = 0;
			_updateLY(0);
			STAT.mode = Mode::Searching;
		}
	}

	++vblankCount;
}

void PPU::_updateLY(u8 y) {
	LY = y;
	STAT.coincidence = y == LYC;
	if (STAT.coincidence && STAT.lycInterrupt) {
		interrupt.requestLcdStat = true;
	}
}

//name is a bit ambigious but checks if we're going to the next line with the amount of cycles
bool PPU::_nextLine() {
	if (cycles >= 114) {
		cycles -= 114;

		_updateLY(LY + 1);
		return true;
	}

	return false;
}

std::array<u8, 2> PPU::_fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset) {
	u16 addr = 0;

	if (tileoffset & 0x80) {
		addr = 0x800;
	}
	else if (!method8000) {
		addr = 0x1000;
	}

	size_t loc = addr + ((tileoffset & 0x7F) * 16) + (yoffset * 2);
	return std::array<u8, 2>{
		VRAM[loc],
		VRAM[loc + 1],
	};
}

std::array<u8, 16> PPU::_fetchTile(u16 addr, u8 tileoffset) {
	size_t loc = (addr - 0x8000) + (tileoffset * 16);
	return std::array<u8, 16>{
		VRAM[loc],
		VRAM[loc + 1], 
		VRAM[loc + 2],
		VRAM[loc + 3],
		VRAM[loc + 4],
		VRAM[loc + 5],
		VRAM[loc + 6],
		VRAM[loc + 7],
		VRAM[loc + 8],
		VRAM[loc + 9],
		VRAM[loc + 10],
		VRAM[loc + 11],
		VRAM[loc + 12],
		VRAM[loc + 13],
		VRAM[loc + 14],
		VRAM[loc + 15]
	};
}

//maybe add support for an auto option?
void PPU::dumpBGMap(std::array<u32, 256 * 256>& outData, bool bgMap, bool tileSet) {
	auto map = VRAM.begin() + ((bgMap) ? 0x1C00 : 0x1800);

	for (int t = 0; t < 0x400; ++t) {
		for (int y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(tileSet, y, map[t]);

			size_t rgbIndex = ((t / 32ll) * 256 * 8) + ((t % 32ll) * 8ll) + (y * 256ll);
			rowHelper(outData, rgbIndex, tile[0], tile[1]);
		}
	}
}

void PPU::dumpTileMap(std::array<u32, 128 * 64 * 3>& outData) {
	for (int t = 0; t < 0x180; ++t) {
		for (int i = 0; i < 8; ++i) {
			u8 top = VRAM[(t * 16ll) + (i * 2ll)];
			u8 bottom = VRAM[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = ((t / 16ll) * 128 * 8) + ((t % 16ll) * 8ll) + (i * 128ll);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}

void PPU::dumpSprites(std::array<u32, 64 * 40>& outData) {
	for (u8 obj = 0; obj < 40; ++obj) {
		for (u8 y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(true, y, sprites[obj].tile);

			size_t rgbIndex = ((obj / 8ll) * 64 * 8) + ((obj % 8) * 8ll) + (y * 64ll);
			rowHelper(outData, rgbIndex, tile[0], tile[1], true, (sprites[obj].useOBP1) ? OBP1 : OBP0);
		}
	}
}