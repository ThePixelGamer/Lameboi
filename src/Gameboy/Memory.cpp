#include "Memory.h"

#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::end

#include <fmt/format.h>

Memory::Memory(Gameboy& t_gb) : gb(t_gb) {
	clean();
}

void Memory::clean() {
	WRAMBank0.fill(0);
	WRAMBank1.fill(0);
	mirrorWRAM.fill(0);
	DMA = 0;
	boot = true;
	undocumented.fill(0);
	HRAM.fill(0);

	memoryRead = false;
	dmaAddr = 0;
	dmaOffset = 0;
}

void Memory::update() {
	if (dmaOffset == 0) {
		return;
	}

	if (dmaOffset <= 0xA0) {
		u8 idx = (0xA0 - dmaOffset);
		dmaAddr = (DMA << 8) | idx;
		gb.ppu.writeOAM(idx, _read(dmaAddr), true);
	}

	--dmaOffset;
}

u8 Memory::read(u16 loc) {
	if (loc == 0xFFFF) {
		return gb.interrupt.read(0xFF);
	}
	else if (loc >= 0xFF80) {
		return HRAM[loc - 0xFF80];
	}
	else if (loc == 0xFF0F) {
		return gb.interrupt.read(0x0F);
	}
	else if (loc >= 0xFF00) {
		u8 ioreg = loc & 0x7F;
		if (ioreg == 0x46) {
			return DMA;
		}
		else {
			return gb.io.read(ioreg);
		}
	}
	else if (dmaOffset != 0) {
		if (dmaOffset <= 0xA0 && loc >= 0xFE00) {
			return 0xFF;
		}

		u8 byte = 0;
		if (inRange(loc, 0x8000, 0x9FFF) != inRange(dmaAddr, 0x8000, 0x9FFF)) {
			byte = _read(loc);
		}
		else {
			byte = _read(dmaAddr);
		}
		return byte;
	}
	else if (loc >= 0xFEA0) {
		return undocumented[loc - 0xFEA0];
	}
	else if (loc >= 0xFE00) {
		return gb.ppu.readOAM(loc - 0xFE00);
	}
	else {
		return _read(loc);
	}
}

u8 Memory::_read(u16 loc) {
	if (loc >= 0xE000) {
		return mirrorWRAM[loc - 0xE000];
	}
	else if (loc >= 0xD000) {
		return WRAMBank1[loc - 0xD000];
	}
	else if (loc >= 0xC000) {
		return WRAMBank0[loc - 0xC000];
	}
	else if (loc >= 0xA000) { // External RAM
		if (gb.mbc)
			return gb.mbc->read(loc);
	}
	else if (loc >= 0x8000) {
		return gb.ppu.readVRAM(loc - 0x8000);
	}
	else { // ROM
		if (boot && (loc < 0x100)) {
			return gb.bios[loc];
		}

		if (gb.mbc)
			return gb.mbc->read(loc);
	}

	return 0xFF;
}

void Memory::write(u16 loc, u8 value) {
	if (loc == 0xFFFF) {
		gb.interrupt.write(0xFF, value);
	}
	else if (loc >= 0xFF80) {
		HRAM[loc - 0xFF80] = value;
	}
	else if (loc == 0xFF0F) {
		gb.interrupt.write(0x0F, value);
	}
	else if (loc >= 0xFF00) {
		u8 ioreg = loc & 0x7F;
		if (ioreg == 0x46) { // DMA Transfer
			if (dmaOffset >= 0xA0) {
				return;
			}

			DMA = value;
			dmaOffset = 0xA2;
		}
		else {
			gb.io.write(ioreg, value);
		}
	}
	else if (dmaOffset != 0 && dmaOffset <= 0xA0) {
		return;
	}
	else if (loc >= 0xFEA0) { 
		//tetris does this so I may remove it in order to avoid annoyances
		fmt::print("Writing to unused memory [{:x}]: {}\n", loc, value);
	}
	else if (loc >= 0xFE00) {
		gb.ppu.writeOAM(loc - 0xFE00, value);
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
		gb.ppu.writeVRAM(loc - 0x8000, value);
	}
	else {
		if (gb.mbc)
			gb.mbc->write(loc, value);
	}
}
