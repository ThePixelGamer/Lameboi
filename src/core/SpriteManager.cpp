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
#include "util/Log.h"

namespace fs = std::filesystem;

const std::array<u32, 4> indexColors = {
	0xffffffff,
	0xaaaaaaff,
	0x555555ff,
	0x000000ff,
};

SpriteManager::SpriteManager(PPU& ppu, RomContext& romContext, bool& bios) : ppu(ppu), romContext(romContext), inBios(bios) {
	lastWrite = -1;

	/*
	for (auto& sprite : fs::directory_iterator("profiles/bios")) {
		PixelData image;
		u32 width, height;

		unsigned error = lodepng::decode(image, width, height, sprite.path().string());
		
		// if there's an error, display it and skip
		if (error) {
			LB_ERROR(PPU, "decoder error {}: {}", error, lodepng_error_text(error));
			continue;
		}

		// verify sprite is 8x8 (todo: remove this restriction?)
		if ((width % 8) == 0 || (height % 8) == 0) {
			continue;
		}
		
		// todo: use transparency in custom sprite?
		bool usesIndexColors = false;
		for (int i = 0; i < (8 * 8); ++i) {
			u32 color = Color::toInt(image[i * 4], image[(i * 4) + 1], image[(i * 4) + 2]) | 0xFF;

			if (std::find(indexColors.begin(), indexColors.end(), color) == indexColors.end()) {
				usesIndexColors = false;
				break;
			}
		}

		loadedBiosTiles[std::stoull(sprite.path().filename(), nullptr, 16)] = { image, usesIndexColors };
	}
	*/

	loadProfile(loadedBiosTiles, "profiles/bios/");
	loadProfile(loadedGameTiles, "profiles/Tetris (World) (Rev 1)/");

	for (auto& sprite : fs::directory_iterator("profiles/Tetris (World) (Rev 1)/raw/")) {
		PixelData image;
		u32 width, height;

		unsigned error = lodepng::decode(image, width, height, sprite.path().string());

		// if there's an error, display it and skip
		if (error) {
			LB_ERROR(PPU, "decoder error {}: {}", error, lodepng_error_text(error));
			continue;
		}

		// verify sprite is 8x8 (todo: remove this restriction?)
		if (width != 8 || height != 8) {
			continue;
		}

		// todo: use transparency in custom sprite?
		bool usesIndexColors = false;
		for (int i = 0; i < (8 * 8); ++i) {
			u32 color = Color::toInt(image[i * 4], image[(i * 4) + 1], image[(i * 4) + 2]) | 0xFF;

			if (std::find(indexColors.begin(), indexColors.end(), color) == indexColors.end()) {
				usesIndexColors = false;
				break;
			}
		}

		loadedGameTiles[std::stoull(sprite.path().filename(), nullptr, 16)] = { image, usesIndexColors };
	}
}

/*
TileData::Tile TileData::getTile(u16 tileOffset) {
	std::vector<unsigned char> pixels(8 * 8 * sizeof(u32));

	std::stringstream bytes;
	bytes << std::hex << std::setfill('0');
	u8 bottom = 0;
	for (int i = 0; i != 0x10; ++i) {
		u8 byte = ppu.VRAM[tileOffset + i];

		// overwrite pixels vector
		if (i & 1) {
			for (u8 x = 0; x < 8; ++x) {
				u8 bit = 7 - x;
				u8 colorIdx = (getBit(byte, bit) << 1) | getBit(bottom, bit);
				u32 color = PPU::paletteColors[colorIdx];

				u8 y = (i / 2) * 8 * sizeof(u32);
				u8 x_ = x * sizeof(u32);
				pixels[y + x_ + 0] = color >> 24;
				pixels[y + x_ + 1] = color >> 16;
				pixels[y + x_ + 2] = color >> 8;
				pixels[y + x_ + 3] = 0xFF; // alpha should be 0xFF in color;
			}
		}

		bottom = byte;

		// + is to cast to int, because ss parses u8s as ascii
		bytes << std::setw(2) << +byte;
	}

	return { pixels, bytes.str() };
}
*/

inline size_t rotate_by_at_least_1(size_t __val, int __shift) {
	return (__val >> __shift) | (__val << (64 - __shift));
}

inline size_t hash_len_16(size_t __u, size_t __v) {
	const size_t __mul = 0x9ddfea08eb382d69ULL;
	size_t __a = (__u ^ __v) * __mul;
	__a ^= (__a >> 47);
	size_t __b = (__v ^ __a) * __mul;
	__b ^= (__b >> 47);
	__b *= __mul;
	return __b;
}

// is this the best place to have this code?
void SpriteManager::render(std::array<u32, 160 * 144>& display) {

}

size_t SpriteManager::getTileHash(u16 tileOffset) {
	u64 a = 0, b = 0;

	std::memcpy(&a, &ppu.VRAM[tileOffset], sizeof(u64));
	std::memcpy(&b, &ppu.VRAM[tileOffset + 8], sizeof(u64));

	return (hash_len_16(a, rotate_by_at_least_1(b + 16, 16)) ^ b);
}

SpriteManager::Tile SpriteManager::getTilePixels(u16 tileOffset) {
	Tile tile;
	tile.data.resize(8 * 8 * sizeof(u32));
	tile.usesIndexColors = true;

	for (int i = 0; i != 16; i += 2) {
		u8 bottom = ppu.VRAM[tileOffset + i];
		u8 top = ppu.VRAM[tileOffset + i + 1];

		// overwrite pixels vector
		for (u8 x = 0; x < 8; ++x) {
			u8 bit = 7 - x;
			u8 colorIdx = (getBit(top, bit) << 1) | getBit(bottom, bit);
			u32 color = indexColors[colorIdx];

			u8 y = (i / 2) * 8 * sizeof(u32);
			u8 x_ = x * sizeof(u32);
			tile.data[y + x_ + 0] = color >> 24;
			tile.data[y + x_ + 1] = color >> 16;
			tile.data[y + x_ + 2] = color >> 8;
			tile.data[y + x_ + 3] = 0xFF; // alpha should be 0xFF in color;
		}
	}

	return tile;
}

const SpriteManager::Tile& SpriteManager::getTile(u64 tileHash) {
	
	if (inBios) {
		return loadedBiosTiles.at(tileHash);
	}
	else {
		auto tile = loadedGameTiles.find(tileHash);
		if (tile != loadedGameTiles.end()) {
			return tile->second;
		}
	}

	static Tile fallbackTile = { PixelData(8 * 8 * sizeof(u32), 0xFF), true };
	return fallbackTile;
}

// this function only "dumps" a tile when the game stops writing to that location 
// (in our case we handle this through vram writes, hacky but fixes a lot of bugs from game optimizations)
void SpriteManager::dump(u16 offsetIdx) {
	// check to see if last write idx is in tile data
	// todo: change this so that if the last vram write is a tile it won't be missed
	// todo: use the source location for file structure
	// todo: is there any situations where a game will purposely write to the same tile location twice?
	if ((lastWrite != -1) && (offsetIdx != lastWrite) && (lastWrite < 0x180)) {
		size_t tileHash = getTileHash(lastWrite << 4);
		auto& loadedTiles = (inBios) ? loadedBiosTiles : loadedGameTiles;

		if (loadedTiles.find(tileHash) == loadedTiles.end()) {
			Tile tilePixels = getTilePixels(lastWrite << 4);

			std::stringstream ss;
			ss << std::hex << tileHash;
			std::string tileHashStr = ss.str();

			LB_INFO(PPU, "[{}]: {}", lastWrite, tileHashStr);
			loadedTiles.emplace(tileHash, tilePixels);

			// write vector to png
			std::string folder = ((inBios) ? "bios" : romContext.fileName) + "/raw/";
			unsigned error = lodepng::encode("profiles/" + folder + tileHashStr + ".png", tilePixels.data, 8, 8);

			// if there's an error, display it
			if (error)
				LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
		}
	}

	lastWrite = offsetIdx;
}

void SpriteManager::writeIntercept(u16 offsetIdx) {
	if (autoDumping) {
		dump(offsetIdx);
	}

	// 0x9800-0x9FFF
	// todo: make a tilemap with "marks" for replacing tiles
	if (offsetIdx >= 0x180) {

	}
}

void SpriteManager::loadProfile(std::map<size_t, Tile>& tilemap, const std::string& path) {
	using json = nlohmann::json;

	const std::string manifestPath = path + "manifest.json";
	if (fs::exists(manifestPath) && !fs::is_empty(manifestPath)) {
		std::fstream manifestFile(manifestPath);

		if (!manifestFile.is_open()) {
			LB_ERROR(CG, "Path {} does not contain manifest.", path);
			return;
		}

		json manifest;
		manifestFile >> manifest;

		if (!manifest.contains("sprites")) {
			LB_ERROR(CG, "Manifest missing required sprites object.");
			return;
		}

		for (auto& [file, sprite] : manifest["sprites"].items()) {
			PixelData image;
			u32 width, height;

			unsigned error = lodepng::decode(image, width, height, path + file + ".png");

			// if there's an error, display it and skip
			if (error) {
				LB_ERROR(CG, "lodepng error {}: {}", error, lodepng_error_text(error));
				continue;
			}

			// verify sprite is tile based (todo: remove this restriction?)
			if ((width % 8) != 0 || (height % 8) != 0) {
				continue;
			}

			if (sprite.is_string()) {
				tilemap[std::stoull(sprite.get<std::string>(), nullptr, 16)] = { image, false };
			}
			else if (sprite.is_object()) {
				for (auto& [hash, uv] : sprite.items()) {
					PixelData uvImage;
					uvImage.resize(8 * 8 * sizeof(u32));

					size_t x = uv[0] * 8;
					size_t y = uv[1] * 8;
					for (size_t i = 0; i < 8; i++) {
						auto& it = image.begin() + (x * sizeof(u32)) + ((i + y) * width * sizeof(u32));
						std::copy(it, it + (8 * sizeof(u32)), uvImage.begin() + (i * 8 * sizeof(u32)));
					}

					tilemap[std::stoull(hash, nullptr, 16)] = { uvImage, false };
				}
			}
		}
	}
}
