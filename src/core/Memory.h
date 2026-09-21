#pragma once

#include <array>
#include <span>

#include "Model.h"
#include "util/MemoryMap.h"
#include "util/Types.h"

class Gameboy;

// https://gbdev.io/pandocs/Memory_Map.html
class Memory {
public:
	using RHandler = u8 (*)(void*, addr);
	using WHandler = void (*)(void*, addr, u8);
	static constexpr size_t PAGE_SIZE = 0x1000;
	static constexpr size_t ADDRESS_SPACE = PAGE_SIZE * 0x10;
	static constexpr size_t TAG_SPACE = ADDRESS_SPACE;

	struct alignas(1) BusTag {
		u8 id : 6 = 0;
		bool read : 1;
		bool write : 1;
	};

    enum TAG_TYPES : size_t {
        NOP = PAGE_SIZE * 0, 
		PASS_THROUGH = PAGE_SIZE * 1, 
		CATCH_WRITE = PAGE_SIZE * 2,
		IO = PAGE_SIZE * 3,
		SIZE = PAGE_SIZE * 4
    };

	ReservedSpace addressSpace;

	u8* Mem() { return addressSpace.get(); }
	BusTag* Tags() { return addressSpace.get<BusTag>() + ADDRESS_SPACE; }

	struct Bus {
		void* data = nullptr;
		RHandler read;
		WHandler write;

		bool enable = true;
	};

	struct Chunk {
		void* base;
		size_t pageSize;
		u16 activeBank;
		u16 banks = 1;

		MemoryMap::ReservedSection section;

		void setup(void* b, u8 pages = 1) {
			base = b;
			pageSize = PAGE_SIZE * pages;
			activeBank = std::numeric_limits<u16>::max();
		}

		bool map(MemoryMap& backing, u16 bank) {
			if (bank == activeBank) return false;

			activeBank = bank;
			unmap();
			section = backing.map(base, pageSize * bank, pageSize);

			return true;
		}

		void unmap() {
			section = {};
		}

		operator size_t() const {
			return pageSize * banks;
		}
	};

	std::array<Bus, 1 << 6> buses {};
	u8 busCount = 1;

	// 8 pages for wram and 1 page for hram
	MemoryMap wram_backing {PAGE_SIZE * 9};
	std::array<u32, PAGE_SIZE * 9> wram_sources;
	Chunk wram0, wram1, eram, hram_io;
	MemoryMap tag_backing { TAG_TYPES::SIZE };
	MemoryMap::ReservedSection wram_tags[4];

	Memory();

	void install(Model::Type model);
	
	void clean() {}
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
