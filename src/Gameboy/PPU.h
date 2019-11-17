#pragma once

#include "Util/types.h"
#include <iostream> // std::cerr std::endl
#include <algorithm>  // std::copy
#include <vector> // std::vector

struct Gameboy;
struct Palette;

#define TILE_SIZE 64

struct Color {
	u32 color0 = 0xFFFFFFFF;
	u32 color1 = 0xFFFFFFFF;
	u32 color2 = 0xFFFFFFFF;
	u32 color3 = 0xFFFFFFFF;
};

struct PPU {
	Gameboy& gb;
	//u32 display[32 * 32 * TILE_SIZE];
	//u8 tileDisplay[32 * 32 * 16];
	u32 bgMap1[32 * 32 * TILE_SIZE]; //0x9800
	u32 bgMap2[32 * 32 * TILE_SIZE]; //0x9C00
	u32 tileData[128 * 3 * TILE_SIZE]; //3 128 tile blocks 
	Color BGP, OBP0, OBP1;
	std::vector<u8> lines;

	PPU(Gameboy&);
	void Clean();
	void LineRender();
	void DecodePalette(Palette& paletteData);
	void WriteTileData(u16 byteLoc, u8 value);
	void WriteBGTile(u16 byteLoc, u8 value);

	//work on copying bg tiles to the the screen
};