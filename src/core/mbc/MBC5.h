#pragma once

#include "MBC.h"
#include "../Memory.h"

class MBC5 : public MBC {
	bool rumbleEnabled = false;
	u16 romBank = 0;

    enum TAG_TYPES : size_t {
        RAM_ENABLE = Memory::PAGE_SIZE * 0, 
		ROM_SELECT = Memory::PAGE_SIZE * 2, 
		ROM9_SELECT = Memory::PAGE_SIZE * 3, 
		RAM_SELECT = Memory::PAGE_SIZE * 4,
		NOP = Memory::PAGE_SIZE * 6,
		SIZE = Memory::PAGE_SIZE * 8
    };

	MemoryMap tag_backing{SIZE};
	MemoryMap::ReservedSection rom_tags[4];

public:
	MBC5(Cartridge& hw) : MBC(hw) {
		auto& bus = hw.bus;
		{
			auto tag_data = tag_backing.map();

			auto ram_enable_tag = bus.register_bus(nullptr, enableRam, this);
			auto rom_bank_number_tag = bus.register_bus(nullptr, selectRom, this);
			auto rom_bank9_tag = bus.register_bus(nullptr, selectRom9, this);
			auto ram_bank_number_tag = bus.register_bus(nullptr, selectRam, this);
			
			auto bus_tag = tag_data.get<Memory::BusTag>();
			std::ranges::fill(std::span(bus_tag + RAM_ENABLE, Memory::PAGE_SIZE * 2), ram_enable_tag);
			std::ranges::fill(std::span(bus_tag + ROM_SELECT, Memory::PAGE_SIZE), rom_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + ROM9_SELECT, Memory::PAGE_SIZE), rom_bank9_tag);
			std::ranges::fill(std::span(bus_tag + RAM_SELECT, Memory::PAGE_SIZE * 2), ram_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + NOP, Memory::PAGE_SIZE * 2), Memory::BusTag { .read = false, .write = true });
			
		}

		for (u8 i = 0; i < 4; ++i) {
			size_t size = Memory::PAGE_SIZE * 2;
			size_t offset = i * size;
			bus.addressSpace.split(Memory::ADDRESS_SPACE + offset, size);
			rom_tags[i] = tag_backing.map(bus.Tags() + offset, offset, size);
		}
		
		bus.addressSpace.split(Memory::ADDRESS_SPACE + 0xA000, Memory::PAGE_SIZE);
		bus.addressSpace.split(Memory::ADDRESS_SPACE + 0xB000, Memory::PAGE_SIZE);
		hw.enableRam(false);
	}

	virtual ~MBC5() {
		hw.bus.addressSpace.unsplit(Memory::ADDRESS_SPACE, Memory::PAGE_SIZE * 2 * 4);
	}

	static void enableRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC5*>(d);
		m->hw.enableRam((v & 0xF) == 0xA);
	}

	static void selectRom(void* d, addr a, u8 v) {
		auto m = static_cast<MBC5*>(d);
		m->romBank &= 0x100;
		m->romBank |= v;
		m->hw.switchBank1(m->romBank);
	}

	static void selectRom9(void* d, addr a, u8 v) {
		auto m = static_cast<MBC5*>(d);
		m->romBank &= 0xFF;
		m->romBank |= (v & 0x1) << 8;
		m->hw.switchBank1(m->romBank);
	}

	static void selectRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC5*>(d);
		u8 bank = v & 0xF;
		if (m->hw.has(m->hw.RUMBLE)) {
			m->rumbleEnabled = (bank & 0x8);
			bank &= 0x7;
			// todo: rumble
		}

		if (m->hw.has(m->hw.RAM)) {
			// todo: handle potential case of ramBank set higher than available RAM_BANKS
			m->hw.switchRam(bank);
		}
	}
};
