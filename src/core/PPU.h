#pragma once

#include <array> // std::array
#include <condition_variable>
#include <functional>
#include <mutex>
#include <map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "Memory.h"
#include "Model.h"
#include "util/Color.h"
#include "util/Types.h"
#include "util/Common.h"

namespace ui {
	class BGMapWindow;
}

class Gameboy;
class Memory;
class Debugger;

inline const std::array indexColors = {
	Color(0xff, 0xff, 0xff),
	Color(0xaa, 0xaa, 0xaa),
	Color(0x55, 0x55, 0x55),
	Color(0x00, 0x00, 0x00),
};

struct Tile {
	using Data = std::vector<Color>;

	Data data;
	bool usesIndexColors;

	void set(Data&& image, bool index) {
		data = std::move(image);
		usesIndexColors = index;
	}
};

struct Manifest {
	using json = nlohmann::json;

	inline static const std::string rootFolder = "cg/";

	std::string name;
	std::string folder;

	std::map<u32, std::map<std::string, Tile>> tiles;
	//std::map<u32, std::map<std::string, Sprite>> sprites;
	std::vector<std::string> skins;
	std::string selSkin = "";

	Manifest() = default;

	std::string getRawPath() {
		return rootFolder + "raw/" + folder;
	}

	std::string getPath() {
		return rootFolder + folder;
	}

	Tile* getTile(u32 a) {
		auto tile = tiles.lower_bound(a);
		if (tile != tiles.end() && a >= tile->first) {
			if (a > 8 && (a - tile->first) > 8) {
				return nullptr;
			}

			if (tile->second.contains(selSkin)) {
				return &tile->second.at(selSkin);
			}
			else {
				return &tile->second.at("");
			}
		}

		return nullptr;
	}
	
	void load(const std::string& name);

	void unload() {
		
	}

	void loadSprite(const std::string& skin, const std::string& name, const json& sprite);
	void loadTilemap(const std::string& skin, const std::string& name, const json& tilemap);
	bool validate(json& manifest, const std::string& manifestPath);
};
class Interrupt;

struct alignas(1) Sprite {
	u8 yPos, xPos, tile, flags;

	u8 getCGBPal() { return flags & 0x7; }
	bool getBank() { return flags & 0x8; }
	bool useOBP1() { return flags & 0x10; }
	bool xFlip() { return flags & 0x20; }
	bool yFlip() { return flags & 0x40; }
	bool behindBG() { return flags & 0x80; }

	u8& operator[](size_t i) {
		switch(i) {
			default:
			case 0: return yPos;
			case 1: return xPos;
			case 2: return tile;
			case 3: return flags;
		}
	}
};

struct alignas(1) PaletteData {
	u8 color;

	PaletteData() : color(0b11'10'01'00) {}

	PaletteData& operator=(u8 v) {
		color = v;
		return *this;
	}

	bool operator==(PaletteData& pal) {
		return color == pal.color;
	}

	bool operator!=(PaletteData& pal) {
		return !(*this == pal);
	}

	u8 operator[](u8 idx) const {
		switch (idx) {
			case 0: return color & 0x3;
			case 1: return (color >> 2) & 0x3;
			case 2: return (color >> 4) & 0x3;
			case 3: return (color >> 6) & 0x3;
			default: return 0;
		}
	}

	operator u8&() {
		return color;
	}
};

struct CGBPaletteData {
	u8 color[8];

	u16 operator[](u8 idx) const {
		idx *= 2;
		return color[idx + 1] << 8 | color[idx];
	}
};

struct Pixel {
	// index into palette lut (2 bits DMG, 15 bits CGB)
	u16 color : 15 = 0;

	Color getColor(Model::Type model);
};

struct Metainfo {
	union Palette {
		PaletteData dmg;
		CGBPaletteData cgb;
	};

	struct TileID {
		u32 crc32;
		u32 src;
	};

	struct Pixel {
		u16 idx : 10;
		u16 pal : 3;
		u16 x : 3;
		u16 y : 3;
	};
	
	struct Sprite {
		u16 idx : 10;
		u16 pal : 3;
		u8 x, y, attribute;
	};
	
	std::array<Palette, 8 * 2> palettes;
	std::array<TileID, 32 * 24> tiles;
	std::array<Pixel, 160 * 144> bg;
	std::array<Sprite, 40> oam;
};

struct Framebuffer {
	constexpr static size_t W = 160, H = 144;

	std::array<Pixel, W * H> pixels{};
	Metainfo meta{};
};

class PPU {
public:
	constexpr static size_t T = 8;
	constexpr static size_t VRAM_SIZE = Memory::PAGE_SIZE * 2;

	static Pixel DefaultPixel;
	Manifest bios;
	Manifest game;

	Model::Type model;

private:
	const bool& inBios;
	Gameboy& core;
	Memory& bus;
	Debugger& debug;
	Interrupt& interrupt;

	Memory::BusTag vram_bus;
	Memory::BusTag oam_bus;

	MemoryMap vram_backing, vram_tag_backing;
	MemoryMap::ReservedSection vram_tag;

public:
	// RAM
	MemoryMap::Section vram;
	Memory::Chunk vram_chunk; // 0x8000
	Sprite sprites[40]; // 0xFE00

	// I/O Registers
	struct { //0xFF40 LCDC
		u8 displayPriority : 1;	//(0=Off, 1=On)
		u8 objDisplay : 1;		//(0=Off, 1=On)
		u8 objSize : 1;			//(0=8x8, 1=8x16)
		u8 bgMap : 1;			//(0=9800-9BFF, 1=9C00-9FFF)
		u8 tileSet : 1;			//(0=8800-97FF, 1=8000-8FFF)
		u8 windowDisplay : 1; 	//(0=Off, 1=On)
		u8 windowMap : 1;		//(0=9800-9BFF, 1=9C00-9FFF)
		u8 lcdDisplay : 1;		//(0=Off, 1=On)
	} LCDC;
	struct { //0xFF41 LCDC Status
		u8 mode : 2;
		u8 coincidence : 1;
		u8 hblankInterrupt : 1;
		u8 vblankInterrupt : 1;
		u8 oamInterrupt : 1;
		u8 lycInterrupt : 1;
	} STAT;

	u8 SCY; //0xFF42 Scroll Y
	u8 SCX; //0xFF43 Scroll X
	
private:
	u8 LY; //0xFF44 LCDC Y-Coord
	u8 LYC; //0xFF45 LY Compare

	constexpr static u8 DMA_SIZE = 40 * 4;
	u8 DMA_START; //0xFF46 DMA Transfer and Start Address
	u8 dma = 0; //which byte we're currently copying

	PaletteData BGP; //0xFF47 BG Palette Data
	PaletteData OBP0; //0xFF48 Object Palette 0 Data
	PaletteData OBP1; //0xFF49 Object Palette 1 Data
	u8 WY; //0xFF4A Window Y Position
	u8 WX; //0xFF4B Window X Position

	u8 VBK; // IO/4F VRAM bank

	struct PalleteIndex {
		u8 addr : 6;
		u8 : 1;
		u8 autoInc : 1;
	} BGPI, OBPI;

	u16 srcVDMA, dstVDMA;
	bool modeVDMA;
	u16 curVDMA;
	bool activeVDMA;
	
	bool dmgObjPriority;

	//internal
	int cycles;
	int frameCycles;
	int lastTile;
	bool last_stat;

	u8 spritesScanned;
	u8 loadedSprites;
	std::array<u8, 10> renderSprites; //offset
	
	bool windowYTrigger;
	u16 windowLines;

	std::array<CGBPaletteData, 8> bgColors;
	std::array<CGBPaletteData, 8> objColors;

	// 2-bit pixel display
	std::array<Framebuffer, 3> buffers;
	u8 frontIdx = 0;
	
	struct Spare {
		u8 idx;
		bool update;
	};

	std::atomic<Spare> spare {{1, true}};
	u8 backIdx = 2;

	bool vblankHelper;
	
public:
	std::atomic<bool> redraw;

	// Display Palette
	static inline Palette paletteColors = {
		0x9bbc0f,
		0x8bac0f,
		0x306230,
		0x0f380f
	};

	static inline const auto cgbPaletteColors= []() {
		std::array<Color, 0x8000> lut;

		for (u16 c = 0; c < lut.size(); ++c) {

			int r = c & 0x1f;
			int g = (c >> 5) & 0x1f;
			int b = (c >> 10) & 0x1f;
			
			r = (r << 3) | (r >> 2);
			g = (g << 3) | (g >> 2);
			b = (b << 3) | (b >> 2);

			lut[c] =  Color(r, g, b);
		}

		return lut;
	}();

	static inline bool windowEnabled = true;
	static inline bool spritesEnabled = true;

	u16 framesPresented;

	//helper for dumpSprites
	inline static Color invisPixel{ u32(0) };
	std::array<u32, VRAM_SIZE * 2> vram_src_locations{};

	enum Mode {
		HBlank,
		VBlank,
		Searching,
		Drawing
	};

	PPU(Gameboy& gb);

	void install(Memory& bus, Model::Type model);

	void clean();
	void update(bool doubleSpeed);
	
	u8 read(u8 reg);
	void write(u8 reg, u8 value);
	void writeIntercept(addr dst, u32 src, u8 data);

	void setMode(Mode mode);
	void setPalette(Palette p) {
		paletteColors = p;
		redraw = true;
	}

	Tile* getTile(u32 a) {
		if (auto tile = game.getTile(a))
			return tile;

		if (auto tile = bios.getTile(a)) {
			return tile;
		}

		return nullptr;
	}

	Manifest& getManifest() { return getManifest(inBios); }
	Manifest& getManifest(bool boot) { return boot ? bios : game; }

	void dumpTiles(u8* outData, const size_t outW, const size_t tW, const size_t tH, const size_t baseOffset);
	void dumpTile(u8* out, const size_t outW, const u16 tileOffset);

	void render(std::function<void (Framebuffer&)> callback);
	void dumpBGMap(std::array<u8, 256 * 256 * 4>& bgmap, bool bgMap, bool tileSet);
	void dumpTileMap(std::array<u8, 128 * 64 * 3 * 4>& tilemap);
	void dumpBGMapTiles(std::array<u8, 32 * 8 * 32 * 8 * 4>& tiles, Pos2 min, Pos2 max, bool bgMap, bool tileSet);
	void dumpTiles(std::array<u8, 32 * 8 * 32 * 8 * 4>& tiles, u32 x1, u32 y1, u32 x2, u32 y2, u32 w, u32 h);
	void dumpSprites(std::array<u8, 64 * 40 * 4>& sprites);

private:
	void scanline();

	void oamScan();
	void hblank();
	void vblank();
	void vdma();

	bool _nextLine();

	std::array<u8, 2> _fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset);
};

inline Color Pixel::getColor(Model::Type model) {
	return ((model == Model::CGB) ? PPU::cgbPaletteColors.data() : PPU::paletteColors.data())[color];
}
