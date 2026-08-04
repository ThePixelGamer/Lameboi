#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <array>

#include <nlohmann/json_fwd.hpp>

#include "util/Color.h"
#include "util/Types.h"

class PPU;
struct Pixel;

// name is a bit confusing; handles dumping sprites as png and loading pngs to override sprite data
class SpriteManager {
public:
	using json = nlohmann::json;

	inline static const std::string rootFolder = "cg/";

	struct Tile {
		using Data = std::vector<Color>;

		Data data;
		bool usesIndexColors;

		void set(Data&& image, bool index) {
			data = std::move(image);
			usesIndexColors = index;
		}
	};

	using SkinMap = std::map<std::string, Tile>;

	struct Manifest {
		std::string name;
		std::string folder;

		std::map<addr, SkinMap> tiles;
		std::map<std::pair<addr, addr>, SkinMap> sprites;
		std::vector<std::string> skins;
		std::string selSkin = "";

		Manifest() = default;

		std::string getRawPath() {
			return rootFolder + "raw/" + folder;
		}

		std::string getPath() {
			return rootFolder + folder;
		}

		Tile* get(addr a) {
			auto tile = tiles.lower_bound(a);
			if (tile != tiles.end() && a >= tile->first) {
				if (a > 8 && (a - tile->first) > 8) {
					return nullptr;
				}

				auto it = tile->second.find(selSkin);
				if (it != tile->second.end()) {
					return &it->second;
				} 

				it = tile->second.find("");
				if (it != tile->second.end()) {
					return &it->second;
				}
			}

			return nullptr;
		}
		
		void load(const std::string& name);
		void loadTilemap(const std::string& skin, const std::string& name, const json& tilemap);
		bool validate(json& manifest,const std::string& manifestPath);
	};

	struct Sprite {
		addr TL;
		u8 W, H;

		addr tileCond;
	};

private:
	PPU& ppu;
	bool& inBoot;
	Manifest bios;
	Manifest game;

	std::vector<Sprite> sprites;
	std::array<addr, 0x2000> vram_src_locations{};
	std::array<u8, 0x2000> vram;

public:
	SpriteManager(PPU& ppu, bool& bios);

	void loadRom(const std::string& romName);
	Manifest& getManifest() { return getManifest(inBoot); }
	Manifest& getManifest(bool boot) { return boot ? bios : game; }

	const Color* renderPixel(const Pixel& pixel);

	Tile* getTile(bool inBios, addr a) {
		if (auto tile = game.get(a))
			return tile;

		if (auto tile = bios.get(a)) {
			return tile;
		}

		return nullptr;
	}

	Tile::Data getTilePixels(u16 tileOffset);
	void dumpTile(addr offset);
	void writeIntercept(addr dst, addr src, u8 data);

	static bool imageIndexColors(const Tile::Data& image);
};
