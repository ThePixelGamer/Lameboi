#pragma once

#include "Cartridge.h"

#include "core/Memory.h"

// Abstract/Bare metal implementation
class MBC {
protected:
	Cartridge& hw;

public:
	MBC(Cartridge& hw) : hw(hw) {}
	virtual ~MBC() = default;
};

class Bare : public MBC {
	MemoryMap::ReservedSection rom_tag[8];

public:
	Bare(Cartridge& hw): MBC(hw) {
		auto& bus = hw.bus;

		for (u8 i = 0; i < 8; ++i)
			rom_tag[i] = bus.tag_backing.map(bus.Tags() + i * Memory::PAGE_SIZE, Memory::CATCH_WRITE, Memory::PAGE_SIZE);

		hw.enableRam(hw.has(Cartridge::RAM));
	}
};
