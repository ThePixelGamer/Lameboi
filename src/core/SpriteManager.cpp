#include "SpriteManager.h"

#include <iomanip>
#include <filesystem>
#include <map>
#include <sstream>

#include <lodepng.h>
#include <nlohmann/json.hpp>

#include "Gameboy.h"
#include "PPU.h"
#include "util/Common.h"
#include "util/FileUtil.h"
#include "util/Log.h"
#include <util/StringUtils.h>

namespace fs = std::filesystem;

const std::array indexColors = {
	Color(0xff, 0xff, 0xff),
	Color(0xaa, 0xaa, 0xaa),
	Color(0x55, 0x55, 0x55),
	Color(0x00, 0x00, 0x00),
};

SpriteManager::SpriteManager(PPU& ppu, bool& inBios) : ppu(ppu), inBoot(inBios) {
	bios.load("lameboi");
}

void SpriteManager::loadRom(const std::string& romName) {
	game.load(romName);
}

const Color* SpriteManager::renderPixel(const Pixel& pixel) {
	// todo: either clear the screen when bios -> game or use the bios as a fallback
	auto pTile = getTile(inBoot, vram_src_locations[pixel.tile]);
	if (!pTile) {
		return nullptr;
	}
	
	auto& tile = *pTile;
	auto col = &tile.data[pixel.x + (pixel.y * 8)];
	if (tile.usesIndexColors) {
		for (u8 i = 0; i < indexColors.size(); ++i) {
			if (indexColors[i] == *col) {
				return &PPU::paletteColors[pixel.palette[i]];
			}
		}
	}
	return col;
}

std::vector<Color> SpriteManager::getTilePixels(u16 tileOffset) {
	std::vector<Color> tile(8 * 8);
	
	for (int i = 0; i != 16; i += 2) {
		u8 bottom = ppu.vram[tileOffset + i];
		u8 top = ppu.vram[tileOffset + i + 1];

		// overwrite pixels vector
		for (u8 x = 0; x < 8; ++x) {
			u8 bit = 7 - x;
			u8 colorIdx = (getBit(top, bit) << 1) | getBit(bottom, bit);

			u8 y = (i / 2) * 8;
			tile[x + y] = indexColors[colorIdx];
		}
	}

	return tile;
}

void SpriteManager::dumpTile(addr offset) {
	auto& map = getManifest();
	addr tileSrc = vram_src_locations[offset];

	if (map.tiles.find(tileSrc) == map.tiles.end()) {
		std::vector<Color> tilePixels = getTilePixels(offset);

		std::stringstream ss;
		ss << std::hex << tileSrc;
		std::string tileSrcStr = ss.str();

		LB_INFO(PPU, "Dumping tile {} with src 0x{}", offset >> 4, tileSrcStr);

		// write vector to png
		unsigned error = lodepng::encode(map.getPath() + tileSrcStr + ".png", (std::vector<u8>&)tilePixels, 8, 8);

		// if there's an error, display it
		if (error)
			LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
	}

}

void SpriteManager::writeIntercept(addr dst, addr src, u8 data) {
	//LB_INFO(CG, "{:04X} = {:04X} {{ {:02X} }}", dst, src, data);

	u16 vramOffset = dst & 0x1FFF;

	vram_src_locations[vramOffset] = src;
	vram[vramOffset] = data;

	// 0x8000 - 0x97FF
	if (vramOffset < 0x1800) {
	}
	// 0x9800 - 0x9FFF
	else {
		for (auto& sprite : sprites) {
			if (sprite.tileCond) {

			}
		}
		// todo: use writes to check for matching logic
	}
}

bool SpriteManager::imageIndexColors(const Tile::Data& image) {
	for (auto& p : image) {
		if (std::count(indexColors.begin(), indexColors.end(), p) == 0) {
			return false;
		}
	}

	return true;
}

void SpriteManager::Manifest::load(const std::string& n) {
	name = n;
	folder = name + "/";
	if (!fs::exists(getPath())) {
		return;
	}

	json manifest;
	if (validate(manifest, getPath() + "manifest.json")) {
		if (manifest.contains("skins")) {
			auto& jsonSkins = manifest["skins"];

			if (jsonSkins.is_array()) {
				for (auto& skin : jsonSkins) {
					if (skin.is_string()) skins.push_back(skin);
				}
			}
		}

		if (manifest.contains("tilemap")) {
			if (skins.empty()) {
				loadTilemap("", getPath(), manifest["tilemap"]);
			}
			else {
				for (auto& skin : skins) {
					if (selSkin.empty()) selSkin = skin;
					std::string path = getPath() + skin + "/";
					if (!fs::exists(path)) {
						path.pop_back();
					}
					loadTilemap(skin, path, manifest["tilemap"]);
				}
			}
		}

		LB_INFO(CG, "Loaded {}", name);
	}
	else {
		LB_ERROR(CG, "Missing manifest for {}", folder);
	}
}

void SpriteManager::Manifest::loadTilemap(const std::string& skin, const std::string& path, const json& tilemap) {
	if (tilemap.is_object()) {
		for (auto& [name, tiles] : tilemap.items())
			loadTilemap(skin, path + name , tiles);
	}

	std::vector<Color> image;
	u32 width, height;

	unsigned error = lodepng::decode((std::vector<u8>&)image, width, height, path + ".png");

	// if there's an error, display it and skip
	if (error) {
		LB_ERROR(CG, "lodepng error {}: {}", error, lodepng_error_text(error));
		return;
	}

	// verify sprite is 8x8 tile based
	if ((width % 8) != 0 || (height % 8) != 0) {
		LB_ERROR(CG, "Tilemap not in 8x8 format", error, lodepng_error_text(error));
		return;
	}

	bool indexed = imageIndexColors(image);
	size_t tileOffset = 0;
	size_t tileWidth = width / 8;
	for (auto& loc_json : tilemap) {
		if (loc_json.is_string()) {
			std::string loc = loc_json;
			if (!loc.empty()) {
				addr a = stringToHex(loc);

				Tile::Data uvImage(8 * 8);

				size_t x = (tileOffset % tileWidth) * 8;
				size_t y = (tileOffset / tileWidth) * 8;
				for (size_t i = 0; i < 8; i++) {
					auto it = image.begin() + x + ((i + y) * width);
					std::copy_n(it, 8, uvImage.begin() + (i * 8));
				}

				auto& tile = tiles[a][skin];
				tile.set(std::move(uvImage), indexed);
			}
		}

		++tileOffset;
	}
}

bool SpriteManager::Manifest::validate(json& manifest, const std::string& manifestPath) {
	if (!fs::exists(manifestPath)) {
		return false;
	}

	std::fstream manifestFile(manifestPath);
	manifestFile >> manifest;

	if (!manifest.contains("tilemap")) {
		LB_ERROR(CG, "Manifest missing required objects.");
		return false;
	}

	return true;
}
