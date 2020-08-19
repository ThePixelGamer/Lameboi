#pragma once

//https://gbdev.gg8.se/wiki/articles/Memory_Map

#include <array>
#include <iostream>
#include <fstream>
#include <mutex>

#include "Util/types.h"
#include "Palette.h"

struct Gameboy;

class IMBC {
public:
	bool ram = false;
	bool battery = false;

	virtual void setup(std::istream& stream) = 0;
	virtual void save() = 0;
	virtual void close() = 0;
	virtual void write(u16 location, u8 data) = 0;
	virtual u8 read(u16 location) = 0;
};

class MBC0 : public IMBC {
	std::array<u8, 0x4000> bank0; // 0x0000
	std::array<u8, 0x4000> bank1; // 0x8000
	std::array<u8, 0x2000> ERAM; // 0x8000

public:
	MBC0() : IMBC() {
		bank0.fill(0);
		bank1.fill(0);
		ERAM.fill(0);
	}
	
	virtual void setup(std::istream& stream) {
		stream.read((char*)bank0.data(), 0x4000);
		stream.read((char*)bank1.data(), 0x4000);
	}

	virtual void save() {}
	virtual void close() {}

	virtual void write(u16 location, u8 data) {
		if (location >= 0xA000) {
			ERAM[location] = data;
		}
	} 

	virtual u8 read(u16 location) {
		if (location >= 0xA000) {
			return ERAM[location - 0xA000];
		}
		else if (location >= 0x4000) { //maybe have this restricted to 4000h-7FFFh?
			return bank1[location - 0x4000];
		}
		else {
			return bank0[location];
		}
	}
};

class MBC3 : public IMBC {
	bool timer = false;
	bool ramEnabled = false;
	u8 romBank = 1; // 1h-7Fh
	u8 ramBank = 0; // 0h-3h = banks, 8h-Ch = RTC registers
	u8 rtcRegister = 0;
	bool ramOrRtc = false; // false = ram, true = rtc

	std::array<std::array<u8, 0x4000>, 0x80> romBanks; // 00h-7Fh
	std::array<std::array<u8, 0x2000>, 0x4> ramBanks; // 00h-03h

	// unimplemented, use std::chrono?
	u8 rtcS = 0;
	u8 rtcM = 0;
	u8 rtcH = 0;
	union {
		struct {
			u16 counter : 9;
			u8 : 5;
			u8 halt : 1;
			u8 carry : 1;
		};

		u16 rawValue = 0x3E00;
	} rtcD;

public:
	MBC3(bool ram_ = false, bool battery_ = false, bool timer_ = false) : IMBC(),
		timer(timer_) {
		battery = battery_;
		ram = ram_;

		romBanks.fill({});
		ramBanks.fill({});
	}

	std::string _getName() {
		return reinterpret_cast<const char*>(&romBanks[0][0x134]);
	}

	virtual void setup(std::istream& stream) {
		stream.read((char*)romBanks[0].data(), 0x4000);

		//change this to use the size of the rom?
		u16 maxBanks = 0;
		switch (romBanks[0][0x148]) {
			case 0x1: maxBanks = 4; break;
			case 0x5: maxBanks = 64; break;
			default: std::cout << "Max Banks is 0" << std::endl; break;
		}

		// 01h - Amount of Banks
		for (u8 bankAmount = 1; bankAmount < maxBanks; ++bankAmount) {
			stream.read((char*)romBanks[bankAmount].data(), 0x4000);
		}

		//probably change this to only save the ram banks used
		if (battery) {
			std::ifstream file("saves/" + _getName() + ".lbs", std::ios_base::binary);

			for (size_t i = 0; i < ramBanks.size(); ++i) {
				file.read((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void save() {
		// dump ram to file
		if (battery) {
			std::ofstream file("saves/" + _getName() + ".lbs", std::ios_base::binary);

			for (size_t i = 0; i < ramBanks.size(); ++i) {
				file.write((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void close() {
		save();
	}

	virtual void write(u16 location, u8 data) {
		if (location >= 0xA000) {
			if (ramOrRtc) {
				switch (rtcRegister) {
					case 0x0: rtcS = data; break;
					case 0x1: rtcM = data; break;
					case 0x2: rtcH = data; break;
					case 0x3: rtcD.counter = data; break;
					case 0x4: 
						rtcD.counter &= 0xFF;
						rtcD.counter = (data << 8) & 0x100;
						rtcD.halt = (data >> 6);
						rtcD.carry = (data >> 7);
						break;
					default: break;
				}
			}
			else {
				ramBanks[ramBank][location - 0xA000] = data;
			}
		}
		else if (location >= 0x6000) {
			// update the RTC registers?
		}
		else if (location >= 0x4000) {
			if (data & 0x8) {
				ramOrRtc = true;
				rtcRegister = (data & 0xF) - 8;
			}
			else {
				ramOrRtc = false;
				ramBank = (data & 0x3);
			}
		}
		else if (location >= 0x2000) {
			if (data == 0) {
				romBank = 1;
			}
			else {
				romBank = (data & 0x7F);
			}
		}
	}

	virtual u8 read(u16 location) {
		if (location >= 0xA000) {
			if (ramOrRtc) {
				switch (rtcRegister) {
					case 0x0: return rtcS;
					case 0x1: return rtcM;
					case 0x2: return rtcH;
					case 0x3: return (rtcD.rawValue & 0xFF);
					case 0x4: return (rtcD.rawValue >> 8);
					default: break;
				}
			}
			else {
				return ramBanks[ramBank][location - 0xA000];
			}
		}
		else if (location >= 0x4000) {
			return romBanks[romBank][location - 0x4000];
		}
		else {
			return romBanks[0][location];
		}

		return 0xFF;
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
public:
	std::array<u8, 0x2000> VRAM; //0x8000
	std::array<u8, 0x1000> WRAMBank0; //0xC000
	std::array<u8, 0x1000> WRAMBank1; //0xD000
	std::array<u8, 0x1E00> mirrorWRAM; //0xE000
	union { //0xFE00
		std::array<Sprite, 40> sprites;
		std::array<u8, 0xA0> OAM;
	};
	std::array<u8, 0x60> unusuable; //0xFEA0
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
				u8 clockSelect : 2;		// (00=1024, 01=16, 10=64, 11=256)
				u8 timerOn : 1;	// (0=Off, 1=On)
			} TAC;
			std::array<u8, 0x7> unused2; //0xFF08-0xFF0E Unused
			struct { //0xFF0F Interrupt Flags
				u8 vblank : 1;
				u8 lcdStat : 1;
				u8 timer : 1;
				u8 serial : 1;
				u8 joypad : 1;
			} IF;
			std::array<u8, 0x30> soundStuff;
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
			std::array<u8, 0x6> cgb; //0xFF51-0xFF56 CGB only
			std::array<u8, 0x29> unused4; //0xFF57-0xFF7F Unused only
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
	void resetIO();
	void update(); //oam dma

	void Write(u16, u8);
	u8 Read(u16 loc);

private:
	Gameboy& gb;
	bool memoryRead = false; //bit of a hack to use the read function during dma
	bool inDMA = false;
	u16 currentDMA = 0; //what address the dma was launched with
	u8 DMAOffset = 0; //which byte we're currently copying
};