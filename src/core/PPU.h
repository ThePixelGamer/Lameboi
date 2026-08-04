#pragma once

#include <array> // std::array
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

#include "Memory.h"
#include "util/Color.h"
#include "util/Types.h"
#include "util/Common.h"

namespace ui {
	class BGMapWindow;
}

class Gameboy;
class Memory;
class Debugger;
class SpriteManager;
class Interrupt;

struct alignas(1) Sprite {
	u8 yPos, xPos, tile, flags;

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
	u8 color0 : 2;
	u8 color1 : 2;
	u8 color2 : 2;
	u8 color3 : 2;

	PaletteData() :
		color0(0),
		color1(0),
		color2(0),
		color3(0) {}

	PaletteData(u8 c0, u8 c1, u8 c2, u8 c3) :
		color0(c0),
		color1(c1),
		color2(c2),
		color3(c3) {}

	PaletteData& operator=(u8 c) {
		color0 = (c);
		color1 = (c >> 2);
		color2 = (c >> 4);
		color3 = (c >> 6);
		return *this;
	}

	bool operator==(PaletteData& pal) {
		return color0 == pal.color0
			&& color1 == pal.color1
			&& color2 == pal.color2
			&& color3 == pal.color3;
	}

	bool operator!=(PaletteData& pal) {
		return !(*this == pal);
	}

	u8 operator[](u8 idx) const {
		switch (idx) {
			case 0: return color0;
			case 1: return color1;
			case 2: return color2;
			case 3: return color3;
			default: return 0;
		}
	}

	u8 read() {
		return (color3 << 6) | (color2 << 4) | (color1 << 2) | (color0);
	}
};

struct Pixel {
	static constexpr addr LB_BLANK = 0;

	// gb paletted color
	addr src : 11;
	addr tile : 13;
	u8 x : 3 = 0;
	u8 y : 3 = 0;
	bool inBios : 1 = false;
	bool obj : 1 = false;

	PaletteData palette;
};

class PPU {
public:
	constexpr static size_t W = 160, H = 144;
	constexpr static size_t T = 8;

	static Pixel DefaultPixel;

	struct Framebuffer {
		std::array<Pixel, W * H> metainfo;

		// todo: separate cg and raw color displays
		std::array<u8, W * H> pixels{};
	};

	SpriteManager& spriteManager;

private:
	Memory& bus;
	Debugger& debug;
	Interrupt& interrupt;

	Memory::BusTag vram_bus;
	Memory::BusTag oam_bus;

	MemoryMap vram_backing, vram_tag_backing;
	MemoryMap::ReservedSection vram_tag;

public:
	// RAM
	MemoryMap::ReservedSection vram; //0x8000
	Sprite sprites[40]; //0xFE00

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
	bool redraw;

public:
	// Display Palette
	static inline Palette paletteColors = {
		0x9bbc0f,
		0x8bac0f,
		0x306230,
		0x0f380f
	};

	static inline bool windowEnabled = true;
	static inline bool spritesEnabled = true;

	u16 framesPresented;

	std::mutex vblank_m;

	//helper for dumpSprites
	inline static Color invisPixel{ u32(0) };

	enum Mode {
		HBlank,
		VBlank,
		Searching,
		Drawing
	};

	PPU(Gameboy& gb);

	void clean();
	void update();
	
	u8 read(u8 reg);
	void write(u8 reg, u8 value);
	
	void forceUpdate() {
		redraw = true;
	}

	void setMode(Mode mode);
	void setPalette(Palette p) {
		paletteColors = p;
		forceUpdate();
	}

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

	bool _nextLine();

	u16 _fetchTileAddr(bool method8000, u8 tileoffset);
	std::array<u8, 2> _fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset);
};
