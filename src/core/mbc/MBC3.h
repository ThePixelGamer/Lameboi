#pragma once

#include "MBC.h"
#include "../Memory.h"

class MBC3 : public MBC {
public:
	constexpr static std::size_t RTC_REGS = 0x5;

private:
	// unimplemented, use std::chrono?
	struct RTC {
		// latched data
		u8 S = 0, M = 0, H = 0;
		u16 D = 0;
		bool halt = false, carry = false;

		u8 latchFlag = 0x1;
		u8 reg = 0x0; // 0h-4h = RTC registers

		void write(u8 data) {
			switch (reg) {
				case 0x0: S = data; break;
				case 0x1: M = data; break;
				case 0x2: H = data; break;
				case 0x3: D = (D & 0x100) | data; break;
				case 0x4:
					D = (D & 0xFF) | ((data & 0x1) << 8);
					halt = (data & 0x40);
					carry = (data & 0x80);
					break;
			}
		}

		u8 read() {
			switch (reg) {
				case 0x0: return S;
				case 0x1: return M;
				case 0x2: return H;
				case 0x3: return (D & 0xFF);
				case 0x4:
					return 0x3E | (carry << 7) | (halt << 6) | ((D & 0x100) >> 8);
			}

			return 0x0;
		}

		void use(u8 r) {
			reg = std::max(r, u8(RTC_REGS - 1));
		}

		void latch(u8 data) {
			if (latchFlag == 0x0 && data == 0x1) {
				// todo: latch current time into RTC regs
			}

			latchFlag = data;
		}
	} rtc;

    enum TAG_TYPES : size_t {
        RAM_ENABLE = Memory::PAGE_SIZE * 0, 
		ROM_SELECT = Memory::PAGE_SIZE * 2, 
		RAM_SELECT = Memory::PAGE_SIZE * 4, 
		RTC_LATCH  = Memory::PAGE_SIZE * 6,
		RTC_RW = Memory::PAGE_SIZE * 8,
		SIZE = Memory::PAGE_SIZE * 9
    };

	MemoryMap tag_backing{SIZE};
	MemoryMap::ReservedSection rom_tags[4];

public:
	MBC3(Cartridge& hw) : MBC(hw) {
		auto& bus = hw.bus;
		{
			auto tag_data = tag_backing.map();

			auto ram_enable_tag = bus.register_bus(nullptr, enableRam, this);
			auto rom_bank_number_tag = bus.register_bus(nullptr, selectRom, this);
			auto ram_bank_number_tag = bus.register_bus(nullptr, selectRam, this);
			auto rtc_latch_select_tag = bus.register_bus(nullptr, rtcLatch, this);
			auto rtc_rw_tag = bus.register_bus(
				[](void* d, addr a) -> u8 {
					auto r = static_cast<RTC*>(d);
					return r->read();
				}, 
				[](void* d, addr a, u8 v) {
					auto r = static_cast<RTC*>(d);
					r->write(v);
				}, &rtc);
			
			auto bus_tag = tag_data.get<Memory::BusTag>();
			std::ranges::fill(std::span(bus_tag + RAM_ENABLE, Memory::PAGE_SIZE * 2), ram_enable_tag);
			std::ranges::fill(std::span(bus_tag + ROM_SELECT, Memory::PAGE_SIZE * 2), rom_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + RAM_SELECT, Memory::PAGE_SIZE * 2), ram_bank_number_tag);
			std::ranges::fill(std::span(bus_tag + RTC_LATCH, Memory::PAGE_SIZE * 2), rtc_latch_select_tag);
			std::ranges::fill(std::span(bus_tag + RTC_RW, Memory::PAGE_SIZE), rtc_rw_tag);
		}

		for (u8 i = 0; i < 4; ++i) {
			size_t size = Memory::PAGE_SIZE * 2;
			size_t offset = i * size;
			bus.addressSpace.split(Memory::ADDRESS_SPACE + offset, size);
			rom_tags[i] = tag_backing.map(bus.Tags() + offset, offset, size);
		}
	}

	static void enableRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC3*>(d);
		m->hw.enableRam((v & 0xF) == 0xA);
	}

	static void selectRom(void* d, addr a, u8 v) {
		auto m = static_cast<MBC3*>(d);
		u8 bank = (v & 0x7F);
		m->hw.switchBank1((bank == 0) ? 1 : bank);
	}

	static void selectRam(void* d, addr a, u8 v) {
		auto m = static_cast<MBC3*>(d);
		
		if (v & 0x8) {
			if (m->hw.has(m->hw.TIMER)) {
				m->rtc.use(v & 0x7);
				m->hw.ram0_tag = m->tag_backing.map(m->hw.bus.Tags() + Memory::PAGE_SIZE * 0xA, RTC_RW, Memory::PAGE_SIZE);
				m->hw.ram1_tag = m->tag_backing.map(m->hw.bus.Tags() + Memory::PAGE_SIZE * 0xB, RTC_RW, Memory::PAGE_SIZE);
			}
		}
		else if (m->hw.has(m->hw.RAM)) {
			// todo: handle potential case of ramBank set higher than available RAM_BANKS
			m->hw.switchRam(v & 0x7);
		}
	}

	static void rtcLatch(void* d, addr, u8 v) {
		auto m = static_cast<MBC3*>(d);
		m->rtc.latch(v);
	}
};
