#pragma once

#include "core/Cartridge.h"

// todo: handle MBC1M carts
class MBC1 : public MBC {
	bool altBankMode = false;
	u8 bank1 = 1;
	u8 bank2 = 0;

    enum TAG_TYPES : size_t {
        RAM_ENABLE = Memory::PAGE_SIZE * 0, 
		ROM_SELECT = Memory::PAGE_SIZE * 1, 
		RAM_SELECT = Memory::PAGE_SIZE * 2, 
		MODE_SELECT = Memory::PAGE_SIZE * 3, 
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
			auto ram_bank_number_tag = bus.register_bus(nullptr, selectRam, this);
			auto banking_mode_select_tag = bus.register_bus(nullptr, enableAlt, this);
			
			auto bus_tag = tag_data.get<Memory::BusTag>();
			std::ranges::fill(std::span(bus_tag + RAM_ENABLE, Memory::PAGE_SIZE), ram_enable_tag);
			std::ranges::fill(std::span(bus_tag + ROM_SELECT, Memory::PAGE_SIZE), rom_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + RAM_SELECT, Memory::PAGE_SIZE), ram_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + MODE_SELECT, Memory::PAGE_SIZE), banking_mode_select_tag);
		}

		for (u8 i = 0; i < 4; ++i) {
			size_t size = Memory::PAGE_SIZE;
			size_t offset = i * size;
			rom_tags[i * 2] = tag_backing.map(bus.Tags() + (offset * 2), offset, size);
			rom_tags[(i * 2) + 1] = tag_backing.map(bus.Tags() + (offset * 2) + size, offset, size);
		}

		hw.enableRam(hw.has(hw.RAM));
	}

	void switchBank1() {
		hw.switchBank1((bank2 << 5) | bank1);
	}

	void switchRam() {
		hw.switchRam((altBankMode) ? bank2 : 0);
	}

	void updateBanks() {
		hw.switchBank0((altBankMode) ? (bank2 << 5) : 0);
		switchBank1();
		switchRam();
	}

	static void enableRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC1*>(d);
		m->hw.enableRam((v & 0xF) == 0xA);
		m->switchRam();
	}

	static void selectRom(void* d, addr a, u8 v) {
		auto m = static_cast<MBC1*>(d);
		u8 bank = (v & 0x1F);
		m->bank1 = (bank == 0) ? 1 : bank;
		m->switchBank1();
	}

	static void selectRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC1*>(d);
		m->bank2 = v & 0x3;
		m->updateBanks();
	}
	
	static void enableAlt(void* d, addr a, u8 v) {
		auto m = static_cast<MBC1*>(d);
		m->altBankMode = (v & 0x1);
		m->updateBanks();
	}
};
