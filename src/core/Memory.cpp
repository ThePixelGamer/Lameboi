#include "Memory.h"

#include "Gameboy.h"

#define LOG_NOP_WRITES

u32& Memory::Sources(addr a) {
	switch (a >> 12) {
		case 0x8: case 0x9: return core.ppu.vram_src_locations[PAGE_SIZE * core.ppu.vram_chunk.activeBank + a & 0x1FFF];
		case 0xA: case 0xB: return core.cart.ram_sources[PAGE_SIZE * core.cart.ram0.activeBank + a & 0x1FFF];
		case 0xC: case 0xE: return wram_sources[a & 0xFFF];
		case 0xD: case 0xF: return wram_sources[PAGE_SIZE * wram1.activeBank + a & 0xFFF];
	}
}

Memory::Memory(Gameboy& gb) : core(gb) {
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
	addressSpace.setup(ADDRESS_SPACE + TAG_SPACE);
	
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
	wram0.setup(Mem() + 0xC000, 1);
	wram1.setup(Mem() + 0xD000, 1);
	wram1.banks = 7;
	eram.setup(Mem() + 0xE000, 1);
	hram_io.setup(Mem() + 0xF000, 1);

	wram0.map(wram_backing, 0);
	wram1.map(wram_backing, 1);
	eram.map(wram_backing, 0);
	hram_io.map(wram_backing, 8);
	

	if (model == Model::CGB) {
		auto wram_bank_tag = register_bus(
			nullptr,
			[](void* d, addr a, u8 v) { 
				auto bus = static_cast<Memory*>(d);

				v &= 0x7;
				if (v == 0) v = 1;
				bus->Mem()[0xFF70] = 0xF8 | v;
				bus->wram1.map(bus->wram_backing, v);
			},
			this
		);
		
		register_io(0x70, wram_bank_tag);
	}
}
