#include "Memory.h"

#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::end

#include <fmt/format.h>

Memory::Memory(Gameboy& t_gb) : gb(t_gb) {
	clean();
}

void Memory::clean() {
	WRAM.fill(0);
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

// todo: template for debugger?
u8 Memory::read(u16 loc) {
	if (dmaOffset != 0 && loc < 0xFF00) {
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

	if (loc < 0xFE00) {
		return _read(loc);
	}
	else if (loc < 0xFEA0) {
		return gb.ppu.readOAM(loc & 0xFF);
	}
	else if (loc < 0xFF00) {
		return undocumented[loc - 0xFEA0];
	}
	else {
		return read_high(loc & 0xFF);
	}
}

u8 Memory::_read(u16 loc) {
	u8 type = loc >> 12; //get top 4 bits

	switch (type) {
		case 0x0:
			if (boot && (loc < 0x100)) { // loc < 0x100
				return gb.bios[loc];
			}
			[[fallthrough]];
		
		case 0x1: case 0x2: case 0x3:
		case 0x4: case 0x5: case 0x6: case 0x7:
		case 0xA: case 0xB:
			if (gb.mbc) {
				return gb.mbc->read(loc);
			}
			break;

		case 0x8: case 0x9:
			return gb.ppu.readVRAM(loc & 0x1FFF);

		case 0xC: case 0xD:
		case 0xE: case 0xF:
			return WRAM[loc & 0x1FFF];

		default:
			fmt::print("ur dumb {:x}", loc);
			break;
	}

	return 0xFF;
}

void Memory::write(u16 loc, u8 value) {
	if (dmaOffset != 0 && dmaOffset <= 0xA0 && loc < 0xFF00) {
		return;
	}

	if (loc < 0xFE00) {
		u8 type = loc >> 12; //get top 4 bits

		switch (type) {
			case 0x0: case 0x1: case 0x2: case 0x3:
			case 0x4: case 0x5: case 0x6: case 0x7:
			case 0xA: case 0xB:
				if (gb.mbc) {
					gb.mbc->write(loc, value);
				}
				break;

			case 0x8: case 0x9:
				gb.ppu.writeVRAM(loc & 0x1FFF, value);
				break;

			case 0xC: case 0xD:
			case 0xE: case 0xF:
				WRAM[loc & 0x1FFF] = value;
				break;

			default:
				fmt::print("ur dumb {:x}", loc);
				break;
		}
	}
	else if (loc < 0xFEA0) {
		gb.ppu.writeOAM(loc & 0xFF, value);
	}
	else if (loc < 0xFF00) {
		//tetris does this so I may remove it in order to avoid annoyances
		fmt::print("Writing to unused memory [{:x}]: {}\n", loc, value);
	}
	else {
		write_high(loc & 0xFF, value);
	}
}

u8 Memory::read_high(u8 loc) {
	u8 ioreg = loc & 0x7F;

	if (loc == 0xF || loc == 0xFF) {
		return gb.interrupt.read(loc & 0x80);
	}
	else if (loc & 0x80) {
		return HRAM[loc & 0x7F];
	}
	else if (ioreg == 0x46) {
		return DMA;
	}
	else {
		return gb.io.read(ioreg);
	}
}

void Memory::write_high(u8 loc, u8 value) {
	u8 ioreg = loc & 0x7F;

	if (loc == 0xF || loc == 0xFF) {
		gb.interrupt.write(loc & 0x80, value);
	}
	else if (loc & 0x80) {
		HRAM[loc & 0x7F] = value;
	}
	else if (ioreg == 0x46) { // DMA Transfer
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