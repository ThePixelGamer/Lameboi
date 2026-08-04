#pragma once

#include <array>
#include <span>

#include "util/MemoryMap.h"
#include <util/Types.h>

class Gameboy;

#define LOG_NOP_WRITES

// https://gbdev.io/pandocs/Memory_Map.html
class Memory {
public:
	using RHandler = u8 (*)(void*, addr);
	using WHandler = void (*)(void*, addr, u8);
	static constexpr size_t PAGE_SIZE = 0x1000;
	static constexpr size_t ADDRESS_SPACE = PAGE_SIZE * 0x10;

    enum TAG_TYPES : size_t {
        NOP = PAGE_SIZE * 0, 
		PASS_THROUGH = PAGE_SIZE * 1, 
		CATCH_WRITE = PAGE_SIZE * 2,
		IO = PAGE_SIZE * 3,
		SIZE = PAGE_SIZE * 4
    };

	ReservedSpace addressSpace{ ADDRESS_SPACE * 2 };

	struct alignas(1) BusTag {
		u8 id : 6 = 0;
		bool read : 1;
		bool write : 1;
	};

	struct Bus {
		void* data = nullptr;
		RHandler read;
		WHandler write;

		bool enable = true;
	};

	std::array<Bus, 1 << 6> buses {};
	u8 busCount = 1;

	// 2 pages for wram and 1 page for hram
	MemoryMap wram_backing {PAGE_SIZE * 3};
	MemoryMap::ReservedSection wram, eram;
	MemoryMap::ReservedSection hram_io;
	MemoryMap tag_backing { size_t(TAG_TYPES::SIZE) };
	MemoryMap::ReservedSection wram_tags[4];

	Memory(Gameboy&) {
		buses[0] = {
			.read = [](void*, addr) -> u8 { return 0xFF; },
			.write = [](void*, addr a, u8) {
			#ifdef LOG_NOP_WRITES
				LB_INFO(Memory, "Unhandled write at {:#04X}", a);
			#endif
			}
		};

		// gb address space
		addressSpace.split(0xC000, PAGE_SIZE * 2);
		addressSpace.split(0xE000, PAGE_SIZE);
		addressSpace.split(0xF000, PAGE_SIZE);
		
		// tag space
		addressSpace.split(ADDRESS_SPACE + 0xC000, PAGE_SIZE);
		addressSpace.split(ADDRESS_SPACE + 0xD000, PAGE_SIZE);
		addressSpace.split(ADDRESS_SPACE + 0xE000, PAGE_SIZE);

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
		wram = wram_backing.map(Mem() + 0xC000,  0, PAGE_SIZE * 2);
		eram = wram_backing.map(Mem() + 0xE000,  0, PAGE_SIZE);
		hram_io = wram_backing.map(Mem() + 0xF000, PAGE_SIZE * 2, PAGE_SIZE);
	}
	
	void clean() {}

	u8* Mem() { return addressSpace.get(); }
	BusTag* Tags() { return addressSpace.get<BusTag>() + ADDRESS_SPACE; }

	u8 read(addr loc) {
		auto tag = Tags()[loc];
		auto& bus = buses[tag.id];
		if (!bus.enable) return 0xFF;
		return (tag.read) ? bus.read(bus.data, loc) : Mem()[loc];
	}

	void write(addr loc, u8 value) {
		auto tag = Tags()[loc];
		auto& bus = buses[tag.id];
		if (!bus.enable) return;
		if (tag.write) return bus.write(bus.data, loc, value);
		Mem()[loc] = value;
	}

	BusTag register_bus(RHandler r, WHandler w, void* data) {
		if (busCount == buses.size()) {
			return {};
		}

		buses[busCount] = {
			.data = data,
			.read = r,
			.write = w,

			.enable = true
		};

		return { .id = u8(busCount++), .read = bool(r), .write = bool(w) };
	}

	void register_io(u8 idx, BusTag tag) { Tags()[0xFF00 + idx] = tag; }
};
