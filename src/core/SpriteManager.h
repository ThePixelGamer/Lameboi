#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "util/Types.h"

class PPU;
struct Pixel;
struct RomContext;

// name is a bit confusing; handles dumping sprites as png and loading pngs to override sprite data
class SpriteManager {
	using PixelData = std::vector<unsigned char>;

	struct Tile {
		PixelData data;
		bool usesIndexColors;
		bool rawData;
	};

	using TileMap = std::map<size_t, Tile>;

	PPU& ppu;
	bool& inBios;
	s16 lastWrite;
	TileMap loadedBiosTiles;
	TileMap loadedGameTiles;
	std::string gameFolder;

	bool autoDumping = true;

public:
	SpriteManager(PPU& ppu, bool& bios);

	void loadRom(RomContext& romContext);

	void render(std::array<u32, 160 * 144>& display);
	size_t getTileHash(u16 tileOffset);
	Tile getTilePixels(u16 tileOffset);
	const Tile& getTile(u64 hash, bool inBios);
	const Tile& getTile(const Pixel& pixel);
	void dump(size_t tileHash, TileMap& tileMap);
	void writeIntercept(u16 offsetIdx);

private:
	void loadProfile(std::map<size_t, Tile>& tilemap, const std::string& path);
	bool imageIndexColors(const PixelData& data);
};