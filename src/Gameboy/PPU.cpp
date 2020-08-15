#include "PPU.h"

#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::size
#include <iostream>

PPU::PPU(Gameboy& gb) : gb(gb) {
	display.fill(0);
	bgmap0.fill(0);

	gb.mem.LY = 0x90;
}

void PPU::update(int ticks) {
//BG Map
	for (int i = 0; i < 0x400; ++i) {
		auto offset = gb.mem.Read(((gb.mem.LCDC.bgMap) ? 0x9C00 : 0x9800) + i);
		auto tile = _fetchTile((gb.mem.LCDC.tileSet) ? 0x8000 : 0x8800, offset);
		std::copy(tile.begin(), tile.end(), bgmap0.begin() + (i * 16));
	}
}

std::array<u8, 16> PPU::_fetchTile(u16 addr, u8 offset) {
	u16 loc = addr + (offset * 16);
	return std::array<u8, 16>{
		u8(gb.mem.Read(loc)),
		u8(gb.mem.Read(loc + 1)),
		u8(gb.mem.Read(loc + 2)),
		u8(gb.mem.Read(loc + 3)),
		u8(gb.mem.Read(loc + 4)),
		u8(gb.mem.Read(loc + 5)),
		u8(gb.mem.Read(loc + 6)),
		u8(gb.mem.Read(loc + 7)),
		u8(gb.mem.Read(loc + 8)),
		u8(gb.mem.Read(loc + 9)),
		u8(gb.mem.Read(loc + 10)),
		u8(gb.mem.Read(loc + 11)),
		u8(gb.mem.Read(loc + 12)),
		u8(gb.mem.Read(loc + 13)),
		u8(gb.mem.Read(loc + 14)),
		u8(gb.mem.Read(loc + 15))
	};
}

template<size_t T>
void rowHelper(std::array<u32, T>& outData, size_t index, u8 top, u8 bottom) {
	outData[index]	   = BasePallete[((top & 0x80) >> 6) | ((bottom & 0x80) >> 7)];
	outData[index + 1] = BasePallete[((top & 0x40) >> 5) | ((bottom & 0x40) >> 6)];
	outData[index + 2] = BasePallete[((top & 0x20) >> 4) | ((bottom & 0x20) >> 5)];
	outData[index + 3] = BasePallete[((top & 0x10) >> 3) | ((bottom & 0x10) >> 4)];
	outData[index + 4] = BasePallete[((top & 0x08) >> 2) | ((bottom & 0x08) >> 3)];
	outData[index + 5] = BasePallete[((top & 0x04) >> 1) | ((bottom & 0x04) >> 2)];
	outData[index + 6] = BasePallete[((top & 0x02) >> 0) | ((bottom & 0x02) >> 1)];
	outData[index + 7] = BasePallete[((top & 0x01) << 1) | ((bottom & 0x01) >> 0)];
}

void PPU::dumpBGMap0RGBA(std::array<u32, 256 * 256>& outData) {
	for (int t = 0; t < bgmap0.size() / 16; ++t) {
		for (int y = 0; y < 8; ++y) {
			u8 top = bgmap0[(t * 16ll) + (y * 2ll)];
			u8 bottom = bgmap0[(t * 16ll) + (y * 2ll) + 1];

			size_t rgbIndex = ((t / 32ll) * 256 * 8) + ((t % 32ll) * 8ll) + (y * 256ll);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}

void PPU::dumpTileMap(std::array<u32, 128 * 64 * 3>& outData) {
	for (int t = 0; t < 0x180; ++t) {
		for (int i = 0; i < 8; ++i) {
			u8 top = gb.mem.VRAM[(t * 16ll) + (i * 2ll)];
			u8 bottom = gb.mem.VRAM[(t * 16ll) + (i * 2ll) + 1];

			size_t rgbIndex = ((t / 16ll) * 128 * 8) + ((t % 16ll) * 8ll) + (i * 128ll);
			rowHelper(outData, rgbIndex, top, bottom);
		}
	}
}