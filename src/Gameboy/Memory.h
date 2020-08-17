#pragma once

//https://gbdev.gg8.se/wiki/articles/Memory_Map

#include <array>
#include <iostream>
#include <mutex>

#include "Util/types.h"
#include "Palette.h"

struct Gameboy;

struct IMBC {
	u8* current;
	std::array<u8, 0x4000> romBank0; //0x0000
	
	IMBC() {
		this->current = nullptr;
		romBank0.fill(0);
	}

	virtual void switchBank(u8) = 0;
	virtual void Setup(std::istream&) = 0;
};

struct MBC0 : public IMBC {
	std::array<u8, 0x4000> romBank1;

	MBC0() : IMBC() {
		romBank1.fill(0);
		switchBank(1);
	}
	
	virtual void Setup(std::istream& stream) {
		stream.read((char*)romBank0.data(), 0x4000);
		stream.read((char*)romBank1.data(), 0x4000);
	}

	virtual void switchBank(u8 bank) {
		switch(bank) {
			case 0: break;
			case 1: this->current = romBank1.data(); break;
			default: printf("Switched to unknown bank"); break;
		}
	}
};

struct Sprite {
	u8 yPos, xPos, tile;
	struct {
		u8 : 4; //cgb only
		u8 paletteNum : 1;
		u8 xFlip : 1;
		u8 yFlip : 1;
		u8 priority : 1;
	} flags;
};

class Memory {
	Gameboy& gb;

public:
	std::array<u8, 0x2000> VRAM; //0x8000
	u8 ERAM[0x2000]; //0xA000
	u8 WRAMBank0[0x1000]; //0xC000
	u8 WRAMBank1[0x1000]; //0xD000
	u8 mirrorWRAM[0x1E00]; //0xE000
	union { //0xFE00
		std::array<Sprite, 40> sprites;
		std::array<u8, 0xA0> OAM;
	};
	u8 unusuable[0x60]; //0xFEA0
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
			u8 unused1; //0xFF03 Unused
			u8 DIV; //0xFF04 Divider Register
			u8 TIMA; //0xFF05 Timer Counter
			u8 TMA; //0xFF06 Timer Modulo
			struct { //0xFF07 Timer Control
				u8 clockSelect : 2;		//(0=Off, 1=On)
				u8 timerOn : 1;	//(0=Off, 1=On)
			} TAC;
			std::array<u8, 0x7> unused2; //0xFF08-0xFF0E Unused
			struct { //0xFF0F Interrupt Flags
				u8 vblank : 1;
				u8 lcdStat : 1;
				u8 timer : 1;
				u8 serial : 1;
				u8 joypad : 1;
			} IF;
			u8 soundStuff[0x30];
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
			std::array<u8, 0x4> unused3; //0xFF4C-0xFF4F Unused/CGB only
			struct { //0xFF50 
				u8 BOOT : 1;
			};
			u8 unused4[0x2F]; //0xFF51-0xFF7F Unused/CGB only
			/*
			std::array<u8, 0x6> cgb; //0xFF51-0xFF56 Unused/CGB only
			std::array<u8, 0x29> unused4; //0xFF57-0xFF7F Unused/CGB only
			*/
		};
		u8 IORegs[0x80];
	};
	u8 HRAM[0x7F]; //0xFF80
	union { //0xFFFF
		struct {
			u8 vblank : 1;
			u8 lcdStat : 1;
			u8 timer : 1;
			u8 serial : 1;
			u8 joypad : 1;
		} IE;

		u8 Interrupt;
	};

	Memory(Gameboy&);
	void clean();
	void ResetIORegs();

	void Write(u16, u8);

	u8 Read(u16 loc);
};