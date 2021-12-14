#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "util/Types.h"

class PPU;
struct RomContext;

// name is a bit confusing; handles dumping sprites as png and loading pngs to override sprite data
class SpriteManager {
	using PixelData = std::vector<unsigned char>;

	struct Tile {
		PixelData data;
		bool usesIndexColors;
	};

	PPU& ppu;
	RomContext& romContext;
	bool& inBios;
	s16 lastWrite;
	std::map<size_t, Tile> loadedBiosTiles;
	std::map<size_t, Tile> loadedGameTiles;

	bool autoDumping = true;

public:
	SpriteManager(PPU& ppu, RomContext& romContext, bool& bios);

	void render(std::array<u32, 160 * 144>& display);
	size_t getTileHash(u16 tileOffset);
	Tile getTilePixels(u16 tileOffset);
	const Tile& getTile(u64 tileHash);
	void dump(u16 offsetIdx);
	void writeIntercept(u16 offsetIdx);

private:
	void loadProfile(std::map<size_t, Tile>& tilemap, const std::string& path);
};