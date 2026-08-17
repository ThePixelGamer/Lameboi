#include "Memory.h"

#include "Gameboy.h"

#define LOG_NOP_WRITES

Memory::Memory(Gameboy&) {
	buses[0] = {
		.read = [](void*, addr) -> u8 { return 0xFF; },
		.write = [](void*, addr a, u8) {
		#ifdef LOG_NOP_WRITES
			LB_INFO(Memory, "Unhandled write at 0x{:04X}", a);
		#endif
		}
	};
}

void Memory::install(Model::Type model) {
	addressSpace.setup(ADDRESS_SPACE * 2);
	
	// gb address space
	addressSpace.split(0x0000, PAGE_SIZE * 4);
	addressSpace.split(0x4000, PAGE_SIZE * 4);
	addressSpace.split(0x8000, PAGE_SIZE * 2);
	addressSpace.split(0xA000, PAGE_SIZE * 2);
	addressSpace.split(0xC000, PAGE_SIZE);
	addressSpace.split(0xD000, PAGE_SIZE);
	addressSpace.split(0xE000, PAGE_SIZE);
	addressSpace.split(0xF000, PAGE_SIZE);
	
	// tag space
	for (u8 i = 1; i < 7; ++i)
		addressSpace.split(ADDRESS_SPACE + PAGE_SIZE * i, PAGE_SIZE);
		
	addressSpace.split(ADDRESS_SPACE + 0x8000, PAGE_SIZE * 2);
	addressSpace.split(ADDRESS_SPACE + 0xA000, PAGE_SIZE);
	addressSpace.split(ADDRESS_SPACE + 0xB000, PAGE_SIZE);
	addressSpace.split(ADDRESS_SPACE + 0xC000, PAGE_SIZE);
	addressSpace.split(ADDRESS_SPACE + 0xD000, PAGE_SIZE);
	addressSpace.split(ADDRESS_SPACE + 0xE000, PAGE_SIZE);

	busCount = 1;

	{
		auto tag = tag_backing.map();
		BusTag nop_tag = {.read = true, .write = true};
		BusTag passthrough_tag = {.read = false, .write = false};
		BusTag catch_write_tag = {.read = false, .write = true};
		std::ranges::fill(std::span(tag.get<BusTag>() + NOP, PAGE_SIZE), nop_tag);
		std::ranges::fill(std::span(tag.get<BusTag>() + PASS_THROUGH, PAGE_SIZE), passthrough_tag);
		std::ranges::fill(std::span(tag.get<BusTag>() + CATCH_WRITE, PAGE_SIZE), catch_write_tag);

		auto echo_tag = register_bus(
			[](void* d, addr a) -> u8 { return static_cast<Memory*>(d)->Mem()[a - PAGE_SIZE * 2]; },
			[](void* d, addr a, u8 v) { static_cast<Memory*>(d)->Mem()[a - PAGE_SIZE * 2] = v; },
			this
		);

		// each page is 4KB
		auto io_tags = std::span(tag.get<BusTag>() + IO, PAGE_SIZE);
		// 0xF000 - 0xFDFF
		std::ranges::fill(io_tags.first<0xE00>(), echo_tag);
		// 0xFF00 - 0xFF80 fallback for any unmapped io
		std::ranges::fill(io_tags.subspan<0xF00, 0x80>(), nop_tag);
		std::ranges::fill(io_tags.subspan<0xF80, 0x7F>(), passthrough_tag);
	}

	// Create Tags
	wram_tags[0] = tag_backing.map(Tags() + 0xC000, PASS_THROUGH, PAGE_SIZE);
	wram_tags[1] = tag_backing.map(Tags() + 0xD000, PASS_THROUGH, PAGE_SIZE);
	wram_tags[2] = tag_backing.map(Tags() + 0xE000, PASS_THROUGH, PAGE_SIZE);
	wram_tags[3] = tag_backing.map(Tags() + 0xF000, IO, PAGE_SIZE);

	// Create Wram, Echo, and Hram
	wram = wram_backing.map(Mem() + 0xC000, 0, PAGE_SIZE);
	eram = wram_backing.map(Mem() + 0xE000, 0, PAGE_SIZE);
	hram_io = wram_backing.map(Mem() + 0xF000, PAGE_SIZE * 8, PAGE_SIZE);
	
	switchWRAM(1);

	if (model == Model::CGB) {
		auto wram_bank_tag = register_bus(
			nullptr,
			[](void* d, addr a, u8 v) { static_cast<Memory*>(d)->switchWRAM(v); },
			this
		);
		
		register_io(0x70, wram_bank_tag);
	}
}
