#pragma once

#include "core/Cartridge.h"

#include "core/Memory.h"

class Bare : public MBC {
	MemoryMap::ReservedSection rom_tag[8];

public:
	using MBC::MBC;

	void install() {
		for (u8 i = 0; i < 8; ++i)
			rom_tag[i] = hw.bus.tag_backing.map(hw.bus.Tags() + i * Memory::PAGE_SIZE, Memory::CATCH_WRITE, Memory::PAGE_SIZE);

		hw.enableRam(hw.has(Cartridge::RAM));
	}
};
