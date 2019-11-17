/*
https://gbdev.gg8.se/wiki/articles/Memory_Map
*/

#pragma once

#include "Util/types.h"
#include <iostream> 
#include <mutex>

struct Gameboy;

struct IMBC {
	u8* current;
	u8 romBank0[0x4000]; //0x0000
	
	IMBC() {
		this->current = nullptr;
		std::fill(romBank0, std::end(romBank0), 0);
	}

	virtual void switchBank(u8) = 0;
	virtual void Setup(std::istream&) = 0;
};

struct MBC0 : public IMBC {
	u8 romBank1[0x4000];

	MBC0() : IMBC() {
		std::fill(romBank1, std::end(romBank1), 0);
		switchBank(1);
	}
	
	virtual void Setup(std::istream& stream) {
		stream.read((char*)romBank0, 0x4000);
		stream.read((char*)romBank1, 0x4000);
	}

	virtual void switchBank(u8 bank) {
		switch(bank) {
			case 0: break;
			case 1: this->current = romBank1; break;
			default: printf("lol"); break;
		}
	}
};

struct MBC1 : public IMBC {
	u8 romBank1[0x4000];
	u8 romBank2[0x4000];
	u8 romBank3[0x4000];
	u8 romBank4[0x4000];

	MBC1() : IMBC() {
		std::fill(romBank1, std::end(romBank1), 0);
		std::fill(romBank2, std::end(romBank2), 0);
		std::fill(romBank3, std::end(romBank3), 0);
		std::fill(romBank4, std::end(romBank4), 0);
		switchBank(1);
	}
	
	virtual void Setup(std::istream& stream) {
		stream.read((char*)romBank0, 0x4000);
		stream.read((char*)romBank1, 0x4000);
		stream.read((char*)romBank2, 0x4000);
		stream.read((char*)romBank3, 0x4000);
		stream.read((char*)romBank4, 0x4000);
	}

	virtual void switchBank(u8 bank) {
		switch(bank) {
			case 0: break;
			case 1: this->current = romBank1; break;
			case 2: this->current = romBank2; break;
			case 3: this->current = romBank3; break;
			case 4: this->current = romBank4; break;
			default: printf("lol"); break;
		}
	}
};

struct Palette {
	u8 color0 : 2;
	u8 color1 : 2;
	u8 color2 : 2;
	u8 color3 : 2;

	bool operator==(Palette& pal) {
		return (this == &pal);
	}

	bool operator!=(Palette& pal) {
		return !(this == &pal);
	}
};

struct Memory {
	u8 VRAM[0x2000]; //0x8000
	u8 ERAM[0x2000]; //0xA000
	u8 WRAMBank0[0x1000]; //0xC000
	u8 WRAMBank1[0x1000]; //0xD000
	u8 mirrorWRAM[0x1E00]; //0xE000
	u8 OAM[0xA0]; //0xFE00
	u8 unusuable[0x60]; //0xFEA0
	union {
		struct {
			struct { //0xFF00 joypad
				bool p10 : 1;
				bool p11 : 1;
				bool p12 : 1;
				bool p13 : 1;
				bool selectButton : 1;
				bool selectDirect : 1;
			} joypad;
			u8 serialData; //0xFF01 Serial Data
			struct { //0xFF02 Serial Control 
				bool shiftClock : 1;
				u8 :6;
				bool transferStart : 1;
			} serialControl;
			u8 unused1; //0xFF03 Unused
			u8 DIV; //0xFF04 Divider Register
			u8 TIMA; //0xFF05 Timer Counter
			u8 TMA; //0xFF06 Timer Modulo
			struct { //0xFF07 Timer Control
				bool clockSelect : 2;		//(0=Off, 1=On)
				bool timerOn : 1;	//(0=Off, 1=On)
			} TAC;
			u8 unused2[0x7]; //0xFF08-0xFF0E Unused
			struct { //0xFF0F Interrupt Flags
				bool vblank : 1;
				bool lcdStat : 1;
				bool timer : 1;
				bool serial : 1;
				bool joypad : 1;
			} IF;
			u8 soundStuff[0x30];
			struct { //0xFF40 LCDC
				bool bgDisplay : 1;		//(0=Off, 1=On)
				bool objDisplay : 1;	//(0=Off, 1=On)
				bool objSize : 1;		//(0=8x8, 1=8x16)
				bool bgMap : 1;			//(0=9800-9BFF, 1=9C00-9FFF)
				bool tileSet : 1;		//(0=8800-97FF, 1=8000-8FFF) 0 == false 1 == true
				bool windowDisplay : 1; //(0=Off, 1=On)
				bool windowMap : 1;		//(0=9800-9BFF, 1=9C00-9FFF)
				bool lcdDisplay : 1;	//(0=Off, 1=On)
			} LCDC;
			struct { //0xFF41 LCDC Status
				bool mode : 2;
				bool lycStat : 1;
				bool hblankInterrupt : 1;
				bool vblankInterrupt : 1;
				bool oamInterrupt : 1;
				bool lycInterrupt : 1;
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
			u8 unused3[0x04]; //0xFF4C-0xFF4F Unused/CGB only
			struct { //0xFF50 
				bool off : 1;
			} BOOT;
			u8 unused4[0x2F]; //0xFF51-0xFF7F Unused/CGB only
		};
		u8 IORegs[0x80];
	};
	u8 HRAM[0x7F]; //0xFF80
	u8 IEReg; //0xFFFF

	Gameboy& gb;

	Memory(Gameboy&);
	void Clean();
	void ResetIORegs();

	void Write(u16, u8);
	void Write(u16, u16);

	u16 Read(u16 loc);
};