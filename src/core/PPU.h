#pragma once

#include <array> // std::array
#include <condition_variable>
#include <mutex>
#include <vector>

#include "ppu/Palette.h"
#include "ppu/Sprite.h"
#include "util/Color.h"
#include "util/Types.h"

namespace ui {
	class BGMapWindow;
}

class Gameboy;
class Memory;
class Debugger;
class SpriteManager;
class Interrupt;

struct Pixel {
	static constexpr u64 INVALID_ID = static_cast<u64>(-1);

	// gb paletted color
	u8 color = 0; // 2-bit

	PaletteData palette;
	u64 hash = INVALID_ID; // 8-bytes
	u8 x = 0; // 4-bit
	u8 y = 0; // 4-bit
	bool inBios = false;
};

class PPU {
public:
	static Pixel DefaultPixel;

	struct Framebuffer {
		std::vector<u64> hashes;

		// todo: separate cg and raw color displays
		std::array<Pixel, 160 * 144> pixels;
	};

	SpriteManager& spriteManager;

private:
	//SCX and SCY
	friend ui::BGMapWindow;
	friend SpriteManager;

	Memory& mem;
	Debugger& debug;
	Interrupt& interrupt;

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

	size_t framesPresented;

	std::mutex vblank_m;

	//helper for dumpSprites
	inline static u32 invisPixel = 0;

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

	void render(std::array<u32, 160 * 144>& display);
	void dumpBGMap(std::array<u32, 256 * 256>& bgmap, bool bgMap, bool tileSet);
	void dumpTileMap(std::array<u32, 128 * 64 * 3>& tilemap);
	void dumpTiles(std::array<u32, 32 * 8 * 32 * 8>& tiles, u32 x1, u32 y1, u32 x2, u32 y2, u32 w, u32 h);
	void dumpSprites(std::array<u32, 64 * 40>& sprites);

private:
	void scanline();

	void oamScan();
	void hblank();
	void vblank();

	bool _nextLine();

	u16 _fetchTileAddr(bool method8000, u8 tileoffset);
	std::array<u8, 2> _fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset);
};