#pragma once

//https://gbdev.gg8.se/wiki/articles/Memory_Map

#include <array>
#include <iostream>
#include <mutex>

#include "Util/Types.h"
#include "Palette.h"
#include "Sprite.h"

struct Gameboy;

class Memory {
public:
	std::array<u8, 0x2000> VRAM; //0x8000
	std::array<u8, 0x1000> WRAMBank0; //0xC000
	std::array<u8, 0x1000> WRAMBank1; //0xD000
	std::array<u8, 0x1E00> mirrorWRAM; //0xE000
	union { //0xFE00
		std::array<Sprite, 40> sprites;
		std::array<u8, 0xA0> OAM;
	};
	std::array<u8, 0x60> undocumented; //0xFEA0
	union {
		struct {
			struct { //0xFF00 joypad
				u8 p10 : 1;
				u8 p11 : 1;
				u8 p12 : 1;
				u8 p13 : 1;
				u8 selectDirect : 1;
				u8 selectButton : 1;
			} joypad;
			u8 serialData; //0xFF01 Serial Data
			struct { //0xFF02 Serial Control 
				u8 shiftClock : 1;
				u8 : 6;
				u8 transferStart : 1;
			} serialControl;
			u8 unusedFF03; //0xFF03 Unused
			u8 DIV; //0xFF04 Divider Register
			u8 TIMA; //0xFF05 Timer Counter
			u8 TMA; //0xFF06 Timer Modulo
			struct { //0xFF07 Timer Control
				u8 clockSelect : 2;		// (00=1024, 01=16, 10=64, 11=256)
				u8 timerOn : 1;	// (0=Off, 1=On)
			} TAC;
			std::array<u8, 0x7> unusedFF08; //0xFF08-0xFF0E Unused
			struct { //0xFF0F Interrupt Flags
				u8 vblank : 1;
				u8 lcdStat : 1;
				u8 timer : 1;
				u8 serial : 1;
				u8 joypad : 1;
			} IF;

			//Sound
			std::array<u8, 0x5> channel1;
			u8 unusedFF15;
			std::array<u8, 0x4> channel2;
			std::array<u8, 0x5> channel3;
			u8 unusedFF1F;
			std::array<u8, 0x4> channel4;
			std::array<u8, 0x3> soundControl;

			std::array<u8, 0x9> unusedFF27; //FF27-FF2F
			std::array<u8, 0x10> wavePattern; //FF30-FF3F

			//PPU
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
			u8 DMA; //0xFF46 DMA Transfer and Start Address
			Palette BGP; //0xFF47 BG Palette Data
			Palette OBP0; //0xFF48 Object Palette 0 Data
			Palette OBP1; //0xFF49 Object Palette 1 Data
			u8 WY; //0xFF4A Window Y Position
			u8 WX; //0xFF4B Window X Position
			std::array<u8, 0x4> unusedFF4C; //0xFF4C-0xFF4F Unused/CGB only
			struct { //0xFF50 
				u8 BOOT : 1;
			};
			std::array<u8, 0x6> cgb; //0xFF51-0xFF56 CGB only
			std::array<u8, 0x29> unusedFF57; //0xFF57-0xFF7F Unused only
		};
		std::array<u8, 0x80> IORegs;
	};

	std::array<u8, 0x7F> HRAM; //0xFF80

	union { //0xFFFF
		struct {
			u8 vblank : 1;
			u8 lcdStat : 1;
			u8 timer : 1;
			u8 serial : 1;
			u8 joypad : 1;
		};
		u8 raw;
	} IE;

	Memory(Gameboy&);
	void clean();
	void resetIO();
	void update(); //oam dma

	void Write(u16, u8);
	u8 Read(u16 loc);

private:
	Gameboy& gb;
	bool memoryRead = false; //bit of a hack to use the read function during dma
	bool inDMA = false;
	u16 currentDMA = 0; //what address the dma is currently on
	u8 DMAOffset = 0; //which byte we're currently copying
	u8 DMAByte = 0; //what byte is on the bus for reads below hram
};