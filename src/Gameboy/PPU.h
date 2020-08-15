#pragma once

#include <iostream> // std::cerr std::endl
#include <algorithm> // std::copy
#include <array> // std::array
#include <vector> // std::vector

#include "Util/types.h"

struct Gameboy;

inline std::array<u32, 4> BasePallete {
	0xFFFFFFFF,
	0x7E7E7EFF,
	0x3F3F3FFF,
	0x000000FF
};

class PPU {
	Gameboy& gb;
	friend class Memory;

	std::array<u8, 5760> display;
	std::array<u8, 0x4000> bgmap0;
	std::array<u8, 0x1800> tileData;

public:
	PPU(Gameboy&);
	void update(int ticks);
	
	void dumpBGMap0RGBA(std::array<u32, 256 * 256>& bgmap);
	void dumpTileMap(std::array<u32, 128 * 64 * 3>& tilemap);

private:
	std::array<u8, 16> _fetchTile(u16 addr, u8 offset = 0);
};