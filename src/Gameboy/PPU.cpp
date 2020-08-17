#include "PPU.h"

#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::size
#include <iostream>

PPU::PPU(Gameboy& gb) : mem(gb.mem) {
	clean();
}

void PPU::clean() {
	display.fill(0xFFFFFFFF); //white
	sprites.fill(0);

	cycles = 0;
	lastTile = 0;

	currentSprite = 0;
	spritesScanned = 0;

	drewWindowLine = false;
	windowLines = 0;

	vblankHelper = false;

	vblankCount = 0;
	isVblank = false;

	mem.STAT.mode = Mode::Searching;
}

inline std::array<u32, 4> RGBAFrom2bpp {
	0xFFFFFFFF,
	0xAAAAAAFF,
	0x555555FF,
	0x000000FF
};

u8 GetBit(u8 byte, u8 bit) {
	return (byte & (1 << bit)) != 0;
}

void SetBit(u8& byte, u8 bit, u8 val) {
	byte &= ~(1 << bit);
	byte |= val << bit;
}

template<size_t T>
void rowHelper(std::array<u32, T>& outData, size_t index, u8 bottom, u8 top, bool color0Invis = false, Palette& pallete = Palette(0, 1, 2, 3)) {
	for (int x = 0; x < 8; ++x) {
		u8 bit = 7 - x;
		u8 color = (GetBit(top, bit) << 1) | GetBit(bottom, bit);

		if (color0Invis && color == pallete.color0) {
			outData[index + x] = 0;
		}
		else {
			outData[index + x] = RGBAFrom2bpp[pallete[color]];
		}
	}
}

void PPU::update(int ticks) {
	if (mem.LCDC.lcdDisplay == 0) {
		mem.LY = 0;
		cycles = 0;

		return;
	}

	if (cycles == 0 && mem.LY == 0) {
		mem.STAT.mode = Mode::Searching;
	}

	cycles += ticks;

	switch (mem.STAT.mode) {
		case Mode::Searching:
		{
			//scan 2 sprites for every cycle
			while (spritesScanned < (cycles * 2) && spritesScanned != 40) {
				Sprite sprite = mem.sprites[spritesScanned];

				if (currentSprite != 10 &&
					mem.LY >= (sprite.yPos - 16) && 
					mem.LY < (sprite.yPos - ((mem.LCDC.objSize) ? 0 : 8))) {
					if (sprite.xPos > 0 && sprite.xPos < 167) {
						sprites[currentSprite] = spritesScanned;
					}

					++currentSprite;
				}

				++spritesScanned;
			}

			if (cycles > 20) {
				mem.STAT.mode = Mode::Drawing;
			}

			vblankHelper = false;
		} break;

		case Mode::Drawing:
		{
			if (cycles < 63) {
				return;
			}

			for (u16 x = 0; x < 160; x += 8) {
				std::array<u8, 2> line = { 0, 0 };
				Palette palette = mem.BGP;
						
				//background
				if (mem.LCDC.displayPriority) {
					auto map = mem.VRAM.begin() + ((mem.LCDC.bgMap) ? 0x1C00 : 0x1800);

					u16 adjustedX = x + mem.SCX;
					adjustedX = (adjustedX > 255) ? adjustedX - 256 : adjustedX;

					u16 y = mem.LY + mem.SCY;
					if (y > 255) {
						y -= 256;
					}

					if ((mem.SCX % 8) == 0) {
						u8 offset = map[(adjustedX / 8ll) + ((y / 8ll) * 32)];
						line = _fetchTileLine(mem.LCDC.tileSet, y % 8, offset);
					}
					else {
						u8 xShift = mem.SCX % 8;
						u16 offset = (adjustedX / 8ll) + ((y / 8ll) * 32);
						auto tlineL = _fetchTileLine(mem.LCDC.tileSet, y % 8, map[offset]);
								
						offset = (adjustedX >= 248) ? offset - 31 : offset + 1;
						auto tlineR = _fetchTileLine(mem.LCDC.tileSet, y % 8, map[offset]);
						line = { 
							u8((tlineL[0] << xShift) | (tlineR[0] >> (8 - xShift))),
							u8((tlineL[1] << xShift) | (tlineR[1] >> (8 - xShift))),
						};
					}
				}

				//window
				if (mem.LCDC.displayPriority && mem.LCDC.windowDisplay) {
					auto map = mem.VRAM.begin() + ((mem.LCDC.windowMap) ? 0x1C00 : 0x1800);

					u8 adjustedX = (mem.WX - 7);
					if (x >= adjustedX) {
						if (mem.LY >= mem.WY) {
							u8 offset = map[((x - adjustedX) / 8) + ((windowLines / 8ll) * 32)];
							line = _fetchTileLine(mem.LCDC.tileSet, windowLines % 8, offset);
							drewWindowLine = true;
						}
					}
				}

				//sprites
				if (mem.LCDC.objDisplay) {
					for (u8 i = 0; i < currentSprite; ++i) {
						Sprite sprite = mem.sprites[sprites[i]];
						
						u8 adjustedX = (sprite.xPos - 8);
						if (adjustedX < x || adjustedX >= (x + 8)) {
							continue;
						}

						palette = (sprite.flags.paletteNum) ? mem.OBP1 : mem.OBP0;

						// yPos = 26, LY = 12, spriteSize = 8
						u8 y = (mem.LY + 16 - sprite.yPos) % 16;

						if (sprite.flags.yFlip)
							y = ((mem.LCDC.objSize) ? 15 : 7) - y;

						u8 tileY = y % 8;

						std::array<u8, 2> spriteLine;
						if (mem.LCDC.objSize) { // 8x16
							spriteLine = _fetchTileLine(true, tileY, (y < 8) ? sprite.tile & 0xFE : sprite.tile | 0x1);
						}
						else { // 8x8
							spriteLine = _fetchTileLine(true, tileY, sprite.tile);
						}

						//u8 xShift = 8 - (sprite.xPos % 8);
						spriteLine = {
							u8(spriteLine[0] >> (sprite.xPos % 8)),
							u8(spriteLine[1] >> (sprite.xPos % 8))
						};

						for (u8 i = 0; i < 8; ++i) {
							u8 c0 = GetBit(spriteLine[1], i);
							u8 c1 = GetBit(spriteLine[0], i);
							u8 color = (c0 << 1) | c1;

							if (color != palette.color0) {
								u8 bitX = (sprite.flags.xFlip) ? 8 - i : i;
								SetBit(line[1], bitX, c0);
								SetBit(line[0], bitX, c1);
							}
						}
					}
				}

				rowHelper(display, x + (mem.LY * 160ll), line[0], line[1], false, palette);
			}

			mem.STAT.mode = Mode::HBlank;
		} break;

		case Mode::HBlank:
		{
			if (cycles >= 114) {
				cycles -= 114;

				++mem.LY;
				if (drewWindowLine) {
					drewWindowLine = false;
					++windowLines;
				}
				currentSprite = 0;
				spritesScanned = 0;

				if (mem.LY == mem.LYC) {
					mem.IF.lcdStat = 1;
					mem.STAT.coincidence = 1;
				}
				else {
					mem.STAT.coincidence = 0;
				}

				if (mem.LY == 144) {
					mem.STAT.mode = Mode::VBlank;
				}
				else {
					mem.STAT.mode = Mode::Searching;
				}
			}
		} break;

		case Mode::VBlank:
		{
			if (!vblankHelper) {
				vblankHelper = true;
				mem.IF.vblank = 1;

				std::unique_lock lock(vblank_m);
				vblank.wait(lock, [this] { return isVblank; });
				isVblank = false;
			}

			if (cycles >= 114) {
				cycles -= 114;
					
				if (++mem.LY >= 154) {
					windowLines = 0;
					mem.LY = 0;
					mem.STAT.mode = Mode::Searching;
				}
			}

			++vblankCount;
		} break;

		default:
		{
			throw std::runtime_error("Unknown mode");
		};
	}
}

std::array<u8, 2> PPU::_fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset) {
	u16 addr = 0;

	if (tileoffset & 0x80) {
		addr = 0x800;
	}
	else if (!method8000) {
		addr = 0x1000;
	}

	u16 loc = addr + ((tileoffset & 0x7F) * 16) + (yoffset * 2);
	return std::array<u8, 2>{
		mem.VRAM[loc],
		mem.VRAM[loc + 1ll],
	};
}

std::array<u8, 16> PPU::_fetchTile(u16 addr, u8 tileoffset) {
	u16 loc = (addr - 0x8000) + (tileoffset * 16);
	return std::array<u8, 16>{
		mem.VRAM[loc],
		mem.VRAM[loc + 1ll], 
		mem.VRAM[loc + 2ll],
		mem.VRAM[loc + 3ll],
		mem.VRAM[loc + 4ll],
		mem.VRAM[loc + 5ll],
		mem.VRAM[loc + 6ll],
		mem.VRAM[loc + 7ll],
		mem.VRAM[loc + 8ll],
		mem.VRAM[loc + 9ll],
		mem.VRAM[loc + 10ll],
		mem.VRAM[loc + 11ll],
		mem.VRAM[loc + 12ll],
		mem.VRAM[loc + 13ll],
		mem.VRAM[loc + 14ll],
		mem.VRAM[loc + 15ll]
	};
}

//maybe add support for an auto option?
void PPU::dumpBGMap(std::array<u32, 256 * 256>& outData, bool bgMap, bool tileSet) {
	auto map = mem.VRAM.begin() + ((bgMap) ? 0x1C00 : 0x1800);

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
			u8 top = mem.VRAM[(t * 16ll) + (i * 2ll)];
			u8 bottom = mem.VRAM[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = ((t / 16ll) * 128 * 8) + ((t % 16ll) * 8ll) + (i * 128ll);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}

void PPU::dumpSprites(std::array<u32, 64 * 40>& outData) {
	for (u8 obj = 0; obj < 40; ++obj) {
		for (u8 y = 0; y < 8; ++y) {
			auto tile = _fetchTileLine(true, y, mem.sprites[obj].tile);

			size_t rgbIndex = ((obj / 8ll) * 64 * 8) + ((obj % 8) * 8ll) + (y * 64ll);
			rowHelper(outData, rgbIndex, tile[0], tile[1], false, (mem.sprites[obj].flags.paletteNum) ? mem.OBP1 : mem.OBP0);
		}
	}
}