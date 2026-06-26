#pragma once

#include "MBC.h"

#include <cassert>

class MBC3 : public MBC {
public:
	constexpr static std::size_t RTC_REGS = 0x5;

private:
	bool useRtc = false; 

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

public:

	MBC3(Cartridge& hw) : MBC(hw) {}

	virtual void writeBank0(u16 offset, u8 data) {
		if (offset & 0x2000) {
			romBank = (data & 0x7F);
			if (romBank == 0) {
				romBank = 1;
			}
		}
		else { // 0x0000 - 0x1FFF
			ramEnabled = (data & 0xF) == 0xA;
		}
	};

	virtual void writeBank1(u16 offset, u8 data) {
		if (offset & 0x2000) {
			rtc.latch(data);
		}
		else if (data & 0x8) {
			if (hw.has(hw.TIMER)) {
				useRtc = true;
				rtc.use(data & 0x7);
			}
		}
		else if (hw.has(hw.RAM)) {
			useRtc = false;
			ramBank = data; // handle potential case of ramBank set higher than available RAM_BANKS
		}
	};

	virtual void writeRam(u16 offset, u8 data) {
		if (useRtc) {
			rtc.write(data);
		}
		else {
			MBC::writeRam(offset, data);
		}
	}

	virtual u8 readRam(u16 offset) {
		if (!ramEnabled) {
			return 0xFF;
		}

		if (useRtc) {
			return rtc.read();
		}

		return MBC::readRam(offset);
	}
};