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

namespace fs = std::filesystem;

const std::array<u32, 4> indexColors = {
	0xffffffff,
	0xaaaaaaff,
	0x555555ff,
	0x000000ff,
};

SpriteManager::SpriteManager(PPU& ppu, bool& bios) : ppu(ppu), inBios(bios) {
	loadManifest(biosManifest, "bios/");
}

void SpriteManager::loadRom(RomContext& romContext) {
	gameFolder = romContext.fileName + "/";
	loadManifest(gameManifest, gameFolder);
}

void SpriteManager::render(std::array<u32, 160 * 144>& display) {
	for (size_t p = 0; p < (160 * 144); ++p) {
		auto& pixel = ppu.getBuffer().pixels[p];

		if (pixel.hash != Pixel::INVALID_ID) {
			// todo: either clear the screen when bios -> game or use the bios as a fallback
			auto& tile = ppu.spriteManager.getTile(pixel);
			if (!tile.rawData) {
				size_t pixelIndex = (pixel.x + (pixel.y * 8)) * sizeof(u32);
				u32 color = Color::toInt(tile.data[pixelIndex], tile.data[pixelIndex + 1], tile.data[pixelIndex + 2]) | tile.data[pixelIndex + 3];

				if (tile.usesIndexColors) {
					for (u8 i = 0; i < indexColors.size(); ++i) {
						if (indexColors[i] == color) {
							color = ppu.paletteColors[pixel.palette[i]];
							break;
						}
					}
				}

				display[p] = color;
				continue;
			}
		}

		// fallback if both the hash is invalid or the tile is raw
		display[p] = ppu.paletteColors[pixel.color];
	}
}

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
	tile.rawData = true;

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

const SpriteManager::Tile& SpriteManager::getTile(u64 hash, bool inBios) {
	return getCurrentManifest().getTile(hash);
}

const SpriteManager::Tile& SpriteManager::getTile(const Pixel& pixel) {
	return getTile(pixel.hash, pixel.inBios);
}

SpriteManager::Manifest& SpriteManager::getCurrentManifest() {
	return getManifest(inBios);
}

SpriteManager::Manifest& SpriteManager::getManifest(bool bios) {
	return (bios) ? biosManifest : gameManifest;
}

void SpriteManager::dumpTile(u16 tileOffset, TileMap& tileMap) {
	size_t tileHash = getTileHash(tileOffset << 4);

	if (tileMap.find(tileHash) == tileMap.end()) {
		Tile tilePixels = getTilePixels(tileOffset << 4);

		std::stringstream ss;
		ss << std::hex << tileHash;
		std::string tileHashStr = ss.str();

		LB_INFO(PPU, "[0x{:x}]: {}", tileOffset, tileHashStr);
		tileMap.emplace(tileHash, tilePixels);

		// write vector to png
		std::string folder = "profiles/raw/" + ((inBios) ? "bios/" : gameFolder);
		unsigned error = lodepng::encode(folder + tileHashStr + ".png", tilePixels.data, 8, 8);

		// if there's an error, display it
		if (error)
			LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
	}
}

void SpriteManager::writeIntercept(u16 offsetIdx) {
	u16 tileOffset = offsetIdx >> 4;

	// 0x8000 - 0x97FF
	if (offsetIdx < 0x1800) {
		// dump once we write outside the last tile
		// dmg_boot doesn't use the second color byte when setting up the logo
		if (lastWrite < 0x180 && lastWrite != tileOffset) {
			if (autoDumping) {
				dumpTile(lastWrite, getCurrentManifest().rawTiles);
			}
		}
	}
	// 0x9800 - 0x9FFF
	else {
		// todo: use writes to check for matching logic
	}

	lastWrite = tileOffset;
}

// todo: change logic? assumes 8x8 tile
bool imageIndexColors(const SpriteManager::PixelData& image) {
	for (int i = 0; i < (8 * 8); ++i) {
		u32 color = Color::toInt(image[i * 4], image[(i * 4) + 1], image[(i * 4) + 2]) | 0xFF;

		if (std::find(indexColors.begin(), indexColors.end(), color) == indexColors.end()) {
			return false;
		}
	}

	return true;
}

bool loadImage(SpriteManager::PixelData& image, u32& width, u32& height, const std::string& path) {
	unsigned error = lodepng::decode(image, width, height, path);

	// if there's an error, display it and skip
	if (error) {
		LB_ERROR(CG, "lodepng error {}: {}", error, lodepng_error_text(error));
		return false;
	}

	// verify sprite is tile based 
	//todo: would it be possible to remove this restriction?
	if ((width % 8) != 0 || (height % 8) != 0) {
		return false;
	}

	return true;
}

void addTile(SpriteManager::TileMap& map, const std::string& hashStr, const SpriteManager::PixelData& data, bool raw = false) {
	u64 hash = std::stoull(hashStr, nullptr, 16);
	
	auto& tile = map[hash];
	tile.data = data;
	tile.usesIndexColors = imageIndexColors(data);
	tile.rawData = false;
}

// todo: would it be more performant to precalculate
bool loadSprites(const nlohmann::json& manifest, SpriteManager::Skin& skin, const std::string& root) {
	for (auto& [file, sprite] : manifest["tilemaps"].items()) {
		SpriteManager::PixelData image;
		u32 width, height;

		if (!loadImage(image, width, height,  root + "/" + file + ".png")) {
			continue;
		}

		if (sprite.is_string()) {
			addTile(skin.map, sprite.get<std::string>(), image);
		}
		else if (sprite.is_object()) {
			SpriteManager::PixelData uvImage;
			uvImage.resize(8 * 8 * sizeof(u32));

			for (auto& [hash, uv] : sprite.items()) {
				size_t x = uv[0] * 8;
				size_t y = uv[1] * 8;
				for (size_t i = 0; i < 8; i++) {
					auto& it = image.begin() + (x * sizeof(u32)) + ((i + y) * width * sizeof(u32));
					std::copy(it, it + (8 * sizeof(u32)), uvImage.begin() + (i * 8 * sizeof(u32)));
				}

				addTile(skin.map, hash, uvImage);
			}
		}
	}

	return true;
}

void SpriteManager::loadProfile(Profile& profile, const std::string& localFolder) {
	using json = nlohmann::json;

	// parse texture packs
	const std::string path = "profiles/" + localFolder + profile.name + "/";
	const std::string manifestPath = path + "manifest.json";
	if (fs::exists(manifestPath) && !fs::is_empty(manifestPath)) {
		std::fstream manifestFile(manifestPath);

		if (!manifestFile.is_open()) {
			LB_ERROR(CG, "Path {} does not contain manifest.", path);
			return;
		}

		json manifest;
		manifestFile >> manifest;

		if (!manifest.contains("sprites") && !manifest.contains("tilemaps")) {
			LB_ERROR(CG, "Manifest missing required objects.");
			return;
		}

		if (manifest.contains("skins")) {
			for (auto& skinName : manifest["skins"]) {
				Skin skin(skinName);
				if (loadSprites(manifest, skin, path + skin.name)) {
					profile.loadedSkins.push_back(skin);
				}
			}
		}
		else {
			Skin skin("");
			if (loadSprites(manifest, skin, path)) {
				profile.loadedSkins.push_back(skin);
			}
		}

	}
}

void SpriteManager::loadManifest(Manifest& manifest, const std::string& localFolder) {
	for (auto& entry : fs::directory_iterator("profiles/" + localFolder)) {
		if (entry.is_directory()) {
			Profile profile = entry.path().filename().string();
			LB_INFO(PPU, "Loaded {}", profile.name);
			loadProfile(profile, localFolder);
			manifest.profiles.push_back(profile);
		}
	}

	// parse raw tiles to avoid future dumps
	std::string rawPath = "profiles/raw/" + localFolder;
	createDirectory(rawPath); // ensure no shenanigans
	for (auto& sprite : fs::directory_iterator(rawPath)) {
		PixelData image;
		u32 width, height;

		if (!loadImage(image, width, height, sprite.path().string()) && (width != 8 || height != 8)) {
			continue;
		}

		addTile(gameManifest.rawTiles, sprite.path().filename().string(), image, true);
	}
}
