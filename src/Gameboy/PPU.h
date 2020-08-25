#pragma once

#include <iostream> // std::cerr std::endl
#include <algorithm> // std::copy
#include <array> // std::array
#include <vector> // std::vector
#include <condition_variable>
#include <mutex>

#include "Memory.h"
#include "Util/types.h"

class PPU {
	Memory& mem;

	int cycles;
	int lastTile;

	u8 loadedSprites;
	std::array<u8, 10> sprites; //offset
	
	bool drewWindowLine;
	u16 windowLines;

	bool vblankHelper;

public:
	std::array<u32, 160 * 144> display;
	std::array<u32, 160 * 144> displayPresent;
	bool presenting = false;
	int vblankCount;

	//helper for dumpSprites
	inline static u32 invisPixel = 0;

	bool isVblank;
	std::condition_variable vblank;
	std::mutex vblank_m;

	enum Mode {
		HBlank,
		VBlank,
		Searching,
		Drawing
	};

	PPU(Memory&);
	void clean();
	void update();
	
	void dumpBGMap(std::array<u32, 256 * 256>& bgmap, bool bgMap, bool tileSet);
	void dumpTileMap(std::array<u32, 128 * 64 * 3>& tilemap);
	void dumpSprites(std::array<u32, 64 * 40>& sprites);

private:
	void fifo();
	void scanline();

	bool _nextLine();

	std::array<u8, 16> _fetchTile(u16 addr, u8 tileoffset = 0);
	std::array<u8, 2> _fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset = 0);
};