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
	WRAMBank0.fill(0);
	WRAMBank1.fill(0);
	mirrorWRAM.fill(0);
	OAM.fill(0);
	unusuable.fill(0xFF);
	resetIO();
	HRAM.fill(0);
	IE.raw = 0xE0;
}

/*
This is used to setup the "unimplemented pins" and various default values
*/
void Memory::resetIO() {
	IORegs.fill(0);
	IORegs[0x00] = 0xFF;
	serialData = 0x00;
	IORegs[0x02] = 0x7E; //Serial Control
	IORegs[0x0F] = 0xE0;
	LCDC.tileSet = true;
	LY = 0;

	/*
	STAT.lycInterrupt = true;
	STAT.coincidence = true;
	SCY = 0;
	SCX = 0;
	LY = 0;
	LYC = 0;
	*/

	unusedFF03 = 0xFF;
	unusedFF08.fill(0xFF);
	unusedFF15 = 0xFF;
	unusedFF1F = 0xFF;
	unusedFF27.fill(0xFF);
	unusedFF4C.fill(0xFF);
	unusedFF57.fill(0xFF);
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
		u8 ioreg = loc & 0x7F;
		switch(ioreg) {
			// Valid Writes
			case 0x00: { //joypad
				joypad.selectDirect = (value >> 4);
				joypad.selectButton = (value >> 5);
			} break;
			case 0x02: { //Serial Control
				serialControl.shiftClock = (value);
				serialControl.transferStart = (value >> 7);
			} break;
			case 0x04: {
				DIV = 0;
				gb.scheduler.resetDiv();
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

			case 0x11: {
				gb.apu.channel1.lengthCounter = 64 - (value & 0x3F);

				NR11.soundLength = (value);
				NR11.waveDuty = (value >> 6);
			} break;

			case 0x12: {
				NR12.envelopeSweep = (value);
				NR12.envelopeDirection = (value >> 3);
				NR12.initialVolume = (value >> 4);
			} break;

			case 0x14: { // Channel 1 Frequency HI
                NR14.frequencyHI = (value);
                NR14.counterSelection = (value >> 6);
                NR14.initial = (value >> 7);

				if (NR14.initial) {
					gb.apu.channel1.runEnvelope = true;
					gb.apu.channel1.vol = NR12.initialVolume;
					gb.apu.channel1.shadowFrequency = (NR14.frequencyHI << 8) | NR13;
					
					NR52.sound1On = true;
				}
            } break;

			case 0x16: {
				gb.apu.channel2.lengthCounter = 64 - (value & 0x3F);

				NR21.soundLength = (value);
				NR21.waveDuty = (value >> 6);
			} break;

			case 0x17: {
				NR22.envelopeSweep = (value);
				NR22.envelopeDirection = (value >> 3);
				NR22.initialVolume = (value >> 4);
			} break;

			case 0x19: { // Channel 2 Frequency HI
                NR24.frequencyHI = (value);
                NR24.counterSelection = (value >> 6);
                NR24.initial = (value >> 7);

				if (NR24.initial) {
					gb.apu.channel2.runEnvelope = true;
					gb.apu.channel2.vol = NR22.initialVolume;

					NR52.sound2On = true;
				}
            } break;
                
            case 0x24: { // NR50
                NR50.SO1Volume = (value);
                NR50.vinToS01 = (value >> 3);
                NR50.SO2Volume = (value >> 4);
                NR50.vinToS02 = (value >> 7);
            } break;

            case 0x26: { // NR52
                NR52.soundOn = (value >> 7);
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
			case 0x47: { // BG Palette Data
				BGP = value;
			} break;
			case 0x48: { // OBJ Palette 0 Data
				OBP0 = value;
			} break;
			case 0x49: { // OBJ Palette 1 Data
				OBP1 = value;
			} break;
			case 0x50: { // Boot "boolean"
				BOOT = (value);
			} break;

			// Invalid writes
			case 0x44: break;
			
			// To handle most of the u8 writes
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
		if (gb.mbc)
			gb.mbc->write(loc, value);
	}
	else if (loc >= 0x8000) {
		if (STAT.mode != PPU::Drawing) {
			VRAM[loc - 0x8000] = value;
		}

		//fprintf(log, "Write to VRAM $%04X: $%02X\n", loc, value); //PSI's Logger
	}
	else {
		if (gb.mbc)
			gb.mbc->write(loc, value);
	}
}

u8 Memory::Read(u16 loc) {
	if(loc == 0xFFFF) {
		return IE.raw;
	}
	else if(loc >= 0xFF80) {
		return HRAM[loc - 0xFF80];
	}
	else if(loc >= 0xFF00) {
		u8 ioreg = loc & 0x7F;
		switch (ioreg) {
			case 0x00: {
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
			} break; 
			
			//Write Only bits/registers
			case 0x11: return (NR11.waveDuty << 6) | 0x3F;
			case 0x13: return 0xFF;
			case 0x14: return 0x80 | (NR14.counterSelection << 6) | 0x7;

			case 0x16: return (NR21.waveDuty << 6) | 0x3F;
			case 0x18: return 0xFF;
			case 0x19: return 0x80 | (NR24.counterSelection << 6) | 0x7;
		}

		return IORegs[ioreg];
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
	else if(loc >= 0xA000) { // External RAM
		if (gb.mbc)
			return gb.mbc->read(loc);
	}
	else if(loc >= 0x8000) {
		if (STAT.mode != PPU::Drawing) {
			return VRAM[loc - 0x8000];
		}
	}
	else { // ROM
		if((loc < 0x100) && BOOT == 0) {
			return gb.bios[loc];
		}

		if (gb.mbc)
			return gb.mbc->read(loc);
	}

	return 0xFF;
}