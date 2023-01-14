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
public:
	using PixelData = std::vector<u8>;

	struct Tile {
		PixelData data;
		bool usesIndexColors;
		bool rawData;
	};

	using TileMap = std::map<u64, Tile>;

	struct Skin {
		TileMap map;
		const std::string name;

		Skin(const std::string& _name) : name(_name) {}
	};

	struct Profile {
		std::vector<Skin> loadedSkins;
		size_t currentSkin = 0;
		const std::string name;

		Profile(const std::string& _name) : name(_name) {}

		Skin& getCurrentSkin() {
			return loadedSkins[currentSkin];
		}

		TileMap& getCurrentTileMap() {
			return getCurrentSkin().map;
		}

		Tile* getTile(u64 hash) {
			auto& tiles = getCurrentTileMap();
			auto tile = tiles.find(hash);
			if (tile != tiles.end()) {
				return &tile->second;
			}

			return nullptr;
		}
	};

	struct Manifest {
		TileMap rawTiles;
		std::vector<Profile> profiles;
		size_t currentProfile = 0;

		Profile& getCurrentProfile() {
			return profiles[currentProfile];
		}

		Tile& getTile(u64 hash) {
			auto tile = getCurrentProfile().getTile(hash);
			if (tile) {
				return *tile;
			}

			auto rawTile = rawTiles.find(hash);
			if (rawTile != rawTiles.end()) {
				return rawTile->second;
			}

			static Tile fallbackTile = { PixelData(8 * 8 * sizeof(u32), 0xFF), true };
			return fallbackTile;
		}
	};

private:
	PPU& ppu;
	bool& inBios;
	Manifest biosManifest;
	Manifest gameManifest;
	std::string gameFolder;
	u16 lastWrite = 0xFFFF;
	
	bool autoDumping = true;

public:
	SpriteManager(PPU& ppu, bool& bios);

	void loadRom(RomContext& romContext);

	void render(std::array<u32, 160 * 144>& display);
	size_t getTileHash(u16 tileOffset);
	Tile getTilePixels(u16 tileOffset);
	const Tile& getTile(u64 hash, bool inBios);
	const Tile& getTile(const Pixel& pixel);
	void dumpTile(u16 tileOffset, TileMap& tileMap);
	void writeIntercept(u16 offsetIdx);

	Manifest& getManifest(bool bios);
	void dumpSprite();

private:
	Manifest& getCurrentManifest();

	void loadProfile(Profile& profile, const std::string& path);
	void loadManifest(Manifest& manifest, const std::string& path);
};