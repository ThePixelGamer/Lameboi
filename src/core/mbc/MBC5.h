#pragma once

#include "MBC.h"

class MBC5 : public MBC {
	bool rumbleEnabled = false;
	u16 romBank = 0;

    enum TAG_TYPES : size_t {
        RAM_ENABLE = Memory::PAGE_SIZE * 0, 
		ROM_SELECT = Memory::PAGE_SIZE * 1, 
		ROM9_SELECT = Memory::PAGE_SIZE * 2, 
		RAM_SELECT = Memory::PAGE_SIZE * 3,
		SIZE = Memory::PAGE_SIZE * 4
    };

	MemoryMap tag_backing{SIZE};
	MemoryMap::ReservedSection rom_tags[8];

public:
	using MBC::MBC;

	void install() {
		auto& bus = hw.bus;
		{
			auto tag_data = tag_backing.map();

			auto ram_enable_tag = bus.register_bus(nullptr, enableRam, this);
			auto rom_bank_number_tag = bus.register_bus(nullptr, selectRom, this);
			auto rom_bank9_tag = bus.register_bus(nullptr, selectRom9, this);
			auto ram_bank_number_tag = bus.register_bus(nullptr, selectRam, this);
			
			auto bus_tag = tag_data.get<Memory::BusTag>();
			std::ranges::fill(std::span(bus_tag + RAM_ENABLE, Memory::PAGE_SIZE), ram_enable_tag);
			std::ranges::fill(std::span(bus_tag + ROM_SELECT, Memory::PAGE_SIZE), rom_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + ROM9_SELECT, Memory::PAGE_SIZE), rom_bank9_tag);
			std::ranges::fill(std::span(bus_tag + RAM_SELECT, Memory::PAGE_SIZE), ram_bank_number_tag);
		}

		for (u8 i = 0; i < 4; ++i) {
			size_t size = Memory::PAGE_SIZE;
			size_t offset = i * size;
			rom_tags[i * 2] = tag_backing.map(bus.Tags() + (offset * 2), offset, size);
			rom_tags[(i * 2) + 1] = tag_backing.map(bus.Tags() + (offset * 2) + size, offset, size);
		}
		
		hw.enableRam(false);
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
