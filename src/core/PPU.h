#pragma once

#include <array> // std::array
#include <condition_variable>
#include <mutex>
#include <vector>

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

struct Sprite {
	u8 xPos = 0, yPos = 0, tile = 0;
	bool useOBP1 = false;
	bool xFlip = false, yFlip = false;
	bool behindBG = false;
	u8 lowerX3Byte = 0;

	u8 read(u8 reg) {
		switch (reg) {
			case 0: return yPos;
			case 1: return xPos;
			case 2: return tile;
			case 3: return (behindBG << 7) | (yFlip << 6) | (xFlip << 5) | (useOBP1 << 4) | lowerX3Byte;

			default:
				//log
				return 0xFF;
		}
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
			case 0: yPos = value; break;
			case 1: xPos = value; break;
			case 2: tile = value; break;
			case 3:
				behindBG = (value & 0x80);
				yFlip = (value & 0x40);
				xFlip = (value & 0x20);
				useOBP1 = (value & 0x10);
				lowerX3Byte = (value & 0xF);
				break;

			default:
				//log
				break;
		}
	}
};

struct PaletteData {
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
	static constexpr u64 INVALID_ID = static_cast<u64>(-1);

	// gb paletted color
	PaletteData palette;
	u64 hash = INVALID_ID; // 8-bytes
	u8 x = 0; // 4-bit
	u8 y = 0; // 4-bit
	bool inBios = false;
};

class PPU {
public:
	constexpr static size_t W = 160, H = 144;
	constexpr static size_t T = 8;


	static Pixel DefaultPixel;

	struct Framebuffer {
		std::vector<u64> hashes;
		std::array<Pixel, W * H> metainfo;

		// todo: separate cg and raw color displays
		std::array<u8, W * H> pixels{};
	};

	SpriteManager& spriteManager;

private:
	Memory& mem;
	Debugger& debug;
	Interrupt& interrupt;

public:
	//regs
	std::array<u8, 0x2000> VRAM; //0x8000
	std::array<Sprite, 40> sprites; //0xFE00

	struct { //0xFF40 LCDC
		u8 displayPriority : 1;		//(0=Off, 1=On)
		u8 objDisplay : 1;	//(0=Off, 1=On)
		u8 objSize : 1;		//(0=8x8, 1=8x16)
		u8 bgMap : 1;			//(0=9800-9BFF, 1=9C00-9FFF)
		u8 tileSet : 1;		//(0=8800-97FF, 1=8000-8FFF)
		u8 windowDisplay : 1; //(0=Off, 1=On)
		u8 windowMap : 1;		//(0=9800-9BFF, 1=9C00-9FFF)
		u8 lcdDisplay : 1;	//(0=Off, 1=On)
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
	std::array<Framebuffer, 2> buffers;
	Framebuffer* currentBuffer;
	Framebuffer* nextBuffer;

	bool vblankHelper;

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

	const Framebuffer& getBuffer();
	
	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	u8 readVRAM(u16 offset);
	void writeVRAM(u16 offset, u8 value);

	u8 readOAM(u8 offset);
	void writeOAM(u8 offset, u8 value, bool force = false);

	void dumpTile(u8* out, const size_t outW, const u16 tileOffset);

	void render(std::array<u8, 160 * 144 * 4>& display);
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