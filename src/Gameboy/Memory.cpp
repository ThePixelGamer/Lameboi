#include "Memory.h"
#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::end

Memory::Memory(Gameboy& t_gb) : gb(t_gb) {
	Clean();
}

void Memory::Clean() {
	std::fill(VRAM, std::end(VRAM), 0);
	std::fill(ERAM, std::end(ERAM), 0);
	std::fill(WRAMBank0, std::end(WRAMBank0), 0);
	std::fill(WRAMBank1, std::end(WRAMBank1), 0);
	std::fill(mirrorWRAM, std::end(mirrorWRAM), 0);
	std::fill(OAM, std::end(OAM), 0);
	std::fill(unusuable, std::end(unusuable), 0);
	ResetIORegs();
	std::fill(HRAM, std::end(HRAM), 0);
	IEReg = 0;
}

/*
This is used to setup the "unimplemented pins" and various default values
*/
void Memory::ResetIORegs() {
	std::fill(IORegs, std::end(IORegs), 0);
	IORegs[0x00] = 0xC0; //joypad
	IORegs[0x01] = 0x00; //Serial Data
	IORegs[0x02] = 0x7E; //Serial Control
	IORegs[0x2F] = 0x00; //LCDC
	LCDC.tileSet = true;
	IORegs[0x44] = 0x90; //LY

	{ //Unused/GBC only
	IORegs[0x03] = 0xFF;
	IORegs[0x08] = 0xFF;
	IORegs[0x09] = 0xFF;
	IORegs[0x0A] = 0xFF;
	IORegs[0x0B] = 0xFF;
	IORegs[0x0C] = 0xFF;
	IORegs[0x0D] = 0xFF;
	IORegs[0x0E] = 0xFF;
	IORegs[0x15] = 0xFF;
	IORegs[0x1F] = 0xFF;
	IORegs[0x27] = 0xFF;
	IORegs[0x28] = 0xFF;
	IORegs[0x29] = 0xFF;
	IORegs[0x2A] = 0xFF;
	IORegs[0x2B] = 0xFF;
	IORegs[0x2C] = 0xFF;
	IORegs[0x2D] = 0xFF;
	IORegs[0x2E] = 0xFF;
	IORegs[0x2F] = 0xFF;
	IORegs[0x4C] = 0xFF;
	IORegs[0x4D] = 0xFF;
	IORegs[0x4E] = 0xFF;
	IORegs[0x4F] = 0xFF;
	IORegs[0x57] = 0xFF;
	IORegs[0x58] = 0xFF;
	IORegs[0x59] = 0xFF;
	IORegs[0x5A] = 0xFF;
	IORegs[0x5B] = 0xFF;
	IORegs[0x5C] = 0xFF;
	IORegs[0x5D] = 0xFF;
	IORegs[0x5E] = 0xFF;
	IORegs[0x5F] = 0xFF;
	IORegs[0x60] = 0xFF;
	IORegs[0x61] = 0xFF;
	IORegs[0x62] = 0xFF;
	IORegs[0x63] = 0xFF;
	IORegs[0x64] = 0xFF;
	IORegs[0x65] = 0xFF;
	IORegs[0x66] = 0xFF;
	IORegs[0x67] = 0xFF;
	IORegs[0x6D] = 0xFF;
	IORegs[0x6E] = 0xFF;
	IORegs[0x6F] = 0xFF;
	IORegs[0x70] = 0xFF;
	IORegs[0x71] = 0xFF;
	IORegs[0x78] = 0xFF;
	IORegs[0x79] = 0xFF;
	IORegs[0x7A] = 0xFF;
	IORegs[0x7B] = 0xFF;
	IORegs[0x7C] = 0xFF;
	IORegs[0x7D] = 0xFF;
	IORegs[0x7E] = 0xFF;
	IORegs[0x7F] = 0xFF;
	}
}

void Memory::Write(u16 loc, u8 value) {
	if(loc == 0xFFFF) {
		IEReg = value;
	}
	else if(loc >= 0xFF80) {
		HRAM[loc - 0xFF80] = value;
	}
	else if(loc >= 0xFF00) {
		u8 ioreg = loc & 0xFF;
		switch(ioreg) {
			case 0x00: { //joypad
				joypad.selectButton = (value >> 4);
				joypad.selectDirect = (value >> 5);
			} break;
			case 0x02: { //Serial Control
				serialControl.shiftClock = (value);
				serialControl.transferStart = (value >> 7);
			} break;
			case 0x07: { //Timer Control
				TAC.clockSelect = (value);
				TAC.timerOn = (value >> 2);
			} break;
			case 0x0F: { //Interrupt Flags
				IF.vblank = (value);
				IF.lcdStat = (value >> 1);
				IF.timer = (value >> 2);
				IF.serial = (value >> 3);
				IF.joypad = (value >> 4);
			} break;
			case 0x40: { //LCDC
				LCDC.bgDisplay = (value);
				LCDC.objDisplay = (value >> 1);
				LCDC.objSize = (value >> 2);
				LCDC.bgMap = (value >> 3);
				LCDC.tileSet = (value >> 4);
				LCDC.windowDisplay = (value >> 5);
				LCDC.windowMap = (value >> 6);
				LCDC.lcdDisplay = (value >> 7);
			} break;
			case 0x41: { //LCDC Status
				STAT.mode = (value);
				STAT.lycStat = (value >> 2);
				STAT.hblankInterrupt = (value >> 3);
				STAT.vblankInterrupt = (value >> 4);
				STAT.oamInterrupt = (value >> 5);
				STAT.lycInterrupt = (value >> 6);
			} break;
			case 0x47: { //BG Palette Data
				BGP.color0 = (value);
				BGP.color1 = (value >> 2);
				BGP.color2 = (value >> 4);
				BGP.color3 = (value >> 6);
				gb.ppu.DecodePalette(BGP);
			} break;
			case 0x50: { //BG Palette Data
				BOOT.off = (value);
			} break;

			default: IORegs[loc - 0xFF00] = value; break;
		}
	}
	else if(loc >= 0xFEA0) {
		unusuable[loc - 0xFEA0] = value;
	}
	else if(loc >= 0xFE00) {
		OAM[loc - 0xFE00] = value;
	}
	else if(loc >= 0xE000) {
		mirrorWRAM[loc - 0xE000] = value;
	}
	else if(loc >= 0xD000) {
		WRAMBank1[loc - 0xD000] = value;
	}
	else if(loc >= 0xC000) {
		WRAMBank0[loc - 0xC000] = value;
	}
	else if(loc >= 0xA000) {
		ERAM[loc - 0xA000] = value;
	}
	else if(loc >= 0x8000) {
		VRAM[loc - 0x8000] = value;

		if(loc >= 0x8000 && loc < 0x9800) {
			gb.ppu.WriteTileData(loc, value);
		}
		else if(loc >= 0x9800 && loc < 0xA000) {
			gb.ppu.WriteBGTile(loc, value);
		}

		//fprintf(log, "Write to VRAM $%04X: $%02X\n", loc, value); //PSI's Logger
	}
	else if(loc >= 0x4000) {
		gb.mbc->current[loc - 0x4000] = value;
	}
	else {
		gb.mbc->romBank0[loc] = value;
	}
}

void Memory::Write(u16 loc, u16 value) {
	Write(loc, u8(value & 0xff));
	Write(++loc, u8(value >> 8));
}

u16 Memory::Read(u16 loc) {
	return [&]() {
		if(loc == 0xFFFF) {
			return IEReg;
		}
		else if(loc >= 0xFF80) {
			return HRAM[loc - 0xFF80];
		}
		else if(loc >= 0xFF00) {
			return IORegs[loc - 0xFF00];
		}
		else if(loc >= 0xFEA0) {
			return unusuable[loc - 0xFEA0];
		}
		else if(loc >= 0xFE00) {
			return OAM[loc - 0xFE00];
		}
		else if(loc >= 0xE000) {
			return mirrorWRAM[loc - 0xE000];
		}
		else if(loc >= 0xD000) {
			return WRAMBank1[loc - 0xD000];
		}
		else if(loc >= 0xC000) {
			return WRAMBank0[loc - 0xC000];
		}
		else if(loc >= 0xA000) {
			return ERAM[loc - 0xA000];
		}
		else if(loc >= 0x8000) {
			return VRAM[loc - 0x8000];
		}
		else if(loc >= 0x4000) {
			return gb.mbc->current[loc - 0x4000];
		}
		else {
			if((loc < 0x100) && !BOOT.off) {
				return gb.bios[loc];
			}
			return gb.mbc->romBank0[loc];
		}
	}();
}