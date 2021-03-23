#pragma once

#include <array> // std::array
#include <condition_variable>
#include <mutex>

#include "Interrupt.h"
#include "ppu/Palette.h"
#include "ppu/Sprite.h"
#include "util/Types.h"

namespace ui {
	class BGMapWindow;
}

class PPU {
	//SCX and SCY
	friend ui::BGMapWindow;

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
	Palette BGP; //0xFF47 BG Palette Data
	Palette OBP0; //0xFF48 Object Palette 0 Data
	Palette OBP1; //0xFF49 Object Palette 1 Data
	u8 WY; //0xFF4A Window Y Position
	u8 WX; //0xFF4B Window X Position

	//internal
	int cycles;
	int lastTile;

	u8 spritesScanned;
	u8 loadedSprites;
	std::array<u8, 10> renderSprites; //offset
	
	bool windowYTrigger;
	u16 windowLines;

	bool vblankHelper;

public:
	std::array<u32, 160 * 144> display;
	std::array<u32, 160 * 144> displayPresent;
	bool presenting = false;
	size_t framesPresented;

	bool isVblank;
	std::condition_variable vblank_cv;
	std::mutex vblank_m;

	inline static std::array<u32, 4> ColorPalette {
		0x9BBC0FFF,
		0x8BAC0FFF,
		0x306230FF,
		0x0F380FFF
	};

	//helper for dumpSprites
	inline static u32 invisPixel = 0;

	enum Mode {
		HBlank,
		VBlank,
		Searching,
		Drawing
	};

	PPU(Interrupt& interrupt);

	void clean();
	void update();
	
	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	u8 readVRAM(u16 offset);
	void writeVRAM(u16 offset, u8 value);

	u8 readOAM(u8 offset);
	void writeOAM(u8 offset, u8 value, bool force = false);

	void dumpBGMap(std::array<u32, 256 * 256>& bgmap, bool bgMap, bool tileSet);
	void dumpTileMap(std::array<u32, 128 * 64 * 3>& tilemap);
	void dumpSprites(std::array<u32, 64 * 40>& sprites);

private:
	void fifo();
	void scanline();

	void oamScan();
	void hblank();
	void vblank();

	void _updateLY(u8 y);
	bool _nextLine();

	std::array<u8, 2> _fetchTileLine(bool method8000, u8 yoffset, u8 tileoffset = 0);
};