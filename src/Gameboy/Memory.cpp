#include "Memory.h"

#include "Gameboy.h"
#include "Joypad.h"

#include <algorithm> //std::fill
#include <iterator> //std::end

#include <fmt/format.h>

Memory::Memory(Gameboy& t_gb) : gb(t_gb) {
	clean();
}

void Memory::clean() {
	VRAM.fill(0);
	std::fill(ERAM, std::end(ERAM), 0);
	std::fill(WRAMBank0, std::end(WRAMBank0), 0);
	std::fill(WRAMBank1, std::end(WRAMBank1), 0);
	std::fill(mirrorWRAM, std::end(mirrorWRAM), 0);
	OAM.fill(0);
	std::fill(unusuable, std::end(unusuable), 0xFF);
	resetIO();
	std::fill(HRAM, std::end(HRAM), 0);
	Interrupt = 0xE0;
}

/*
This is used to setup the "unimplemented pins" and various default values
*/
void Memory::resetIO() {
	std::fill(IORegs, std::end(IORegs), 0);
	IORegs[0x00] = 0xFF;
	Write(0xFF01, u8(0x00)); //Serial Data
	Write(0xFF02, u8(0x7E)); //Serial Control
	IORegs[0x0F] = 0xE0;
	LCDC.tileSet = true;
	LY = 0;


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
	/*
	STAT.lycInterrupt = true;
	STAT.coincidence = true;
	SCY = 0;
	SCX = 0;
	LY = 0;
	LYC = 0;

	{ //Unused/GBC only
		unused1 = 0xFF;
		unused2.fill(0xFF);
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
		unused3.fill(0xFF);
		unused4.fill(0xFF);
	}
	*/
}

void Memory::update() {
	if (inDMA) {
		currentDMA = (DMA << 8) | DMAOffset;

		memoryRead = true;
		OAM[DMAOffset] = Read(currentDMA);
		memoryRead = false;

		if (DMAOffset != 0x9F) {
			++DMAOffset;
		}
		else {
			DMAOffset = 0;
			currentDMA = 0;
			inDMA = false;
		}
	}
}

void Memory::Write(u16 loc, u8 value) {
	if (loc == 0xFFFF) {
		IE.vblank = (value >> 0);
		IE.lcdStat = (value >> 1);
		IE.timer = (value >> 2);
		IE.serial = (value >> 3);
		IE.joypad = (value >> 4);
	}
	else if (loc >= 0xFF80) {
		HRAM[loc - 0xFF80] = value;
	}
	else if (loc >= 0xFF00) {
		u8 ioreg = loc & 0xFF;
		switch(ioreg) {
			case 0x00: { //joypad
				joypad.selectDirect = (value >> 4);
				joypad.selectButton = (value >> 5);
			} break;
			case 0x02: { //Serial Control
				serialControl.shiftClock = (value);
				serialControl.transferStart = (value >> 7);
			} break;
			case 0x04: {
				TIMA = 0;
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
				LCDC.displayPriority = (value);
				LCDC.objDisplay = (value >> 1);
				LCDC.objSize = (value >> 2);
				LCDC.bgMap = (value >> 3);
				LCDC.tileSet = (value >> 4);
				LCDC.windowDisplay = (value >> 5);
				LCDC.windowMap = (value >> 6);
				LCDC.lcdDisplay = (value >> 7);
			} break;
			case 0x41: { //LCDC Status
				STAT.hblankInterrupt = (value >> 3);
				STAT.vblankInterrupt = (value >> 4);
				STAT.oamInterrupt = (value >> 5);
				STAT.lycInterrupt = (value >> 6);
			} break;
			case 0x46: { // DMA Transfer
				inDMA = true;
				DMA = value;

				if (currentDMA != 0) {
					DMAOffset = 0;
				}
			} break;
			case 0x47: { //BG Palette Data
				BGP = value;
			} break;
			case 0x50: { //BG Palette Data
				BOOT = (value);
			} break;
			

			default: IORegs[ioreg] = value; break;
		}
	}
	else if (loc >= 0xFEA0) { 
		//tetris does this so I may remove it in order to avoid annoyances
		std::cout << fmt::format("Writing to unused memory [{:x}]: {}\n", loc, value);
	}
	else if (inDMA) {
		return;
	}
	else if (loc >= 0xFE00) {
		if(STAT.mode < 2) { // only allow writes during hblank and vblank 
			OAM[loc - 0xFE00] = value;
		}
	}
	else if (loc >= 0xE000) {
		mirrorWRAM[loc - 0xE000] = value;
	}
	else if (loc >= 0xD000) {
		WRAMBank1[loc - 0xD000] = value;
	}
	else if (loc >= 0xC000) {
		WRAMBank0[loc - 0xC000] = value;
	}
	else if (loc >= 0xA000) {
		ERAM[loc - 0xA000] = value;
	}
	else if (loc >= 0x8000) {
		if (STAT.mode != PPU::Drawing) {
			VRAM[loc - 0x8000] = value;
		}

		//fprintf(log, "Write to VRAM $%04X: $%02X\n", loc, value); //PSI's Logger
	}
	else if (loc >= 0x4000) {
		//gb.mbc->current[loc - 0x4000] = value;
	}
	else {
		//gb.mbc->romBank0[loc] = value;
	}
}

u8 Memory::Read(u16 loc) {
	if(loc == 0xFFFF) {
		return Interrupt;
	}
	else if(loc >= 0xFF80) {
		return HRAM[loc - 0xFF80];
	}
	else if(loc >= 0xFF00) {
		if (loc == 0xFF00) {
			if (joypad.selectButton == 0) {
				joypad.p13 = !gb.pad.getButtonState(Button::Start);
				joypad.p12 = !gb.pad.getButtonState(Button::Select);
				joypad.p11 = !gb.pad.getButtonState(Button::B);
				joypad.p10 = !gb.pad.getButtonState(Button::A);
			}
			else if (joypad.selectDirect == 0) {
				joypad.p13 = !gb.pad.getButtonState(Button::Down);
				joypad.p12 = !gb.pad.getButtonState(Button::Up);
				joypad.p11 = !gb.pad.getButtonState(Button::Left);
				joypad.p10 = !gb.pad.getButtonState(Button::Right);
			}
		}

		return IORegs[loc - 0xFF00];
	}
	else if(loc >= 0xFEA0) {
		return unusuable[loc - 0xFEA0];
	}
	else if (inDMA && !memoryRead) {
		return 0xFF;
	}
	else if(loc >= 0xFE00) {
		if (STAT.mode < 2) {
			return OAM[loc - 0xFE00];
		}
		return 0xFF;
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
		if (STAT.mode != PPU::Drawing) {
			return VRAM[loc - 0x8000];
		}
		return 0xFF;
	}
	else if(loc >= 0x4000) {
		return gb.mbc->current[loc - 0x4000];
	}
	else {
		if((loc < 0x100) && BOOT == 0) {
			return gb.bios[loc];
		}
		return gb.mbc->romBank0[loc];
	}
}