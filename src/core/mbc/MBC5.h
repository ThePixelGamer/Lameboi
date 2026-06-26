#pragma once

#include "MBC.h"

class MBC5 : public MBC {
	bool rumbleEnabled = false;

public:
	MBC5(Cartridge& hw) : MBC(hw) {}

	virtual void writeBank0(u16 offset, u8 data) {
		if (offset & 0x2000) {
			if (offset & 0x1000) {
				romBank &= 0xFF;
				romBank |= (data & 0x1) << 8;
			}
			else {
				romBank &= 0x100;
				romBank |= data;
			}
		}
		else { // 0x0000 - 0x1FFF
			ramEnabled = (data & 0xF) == 0xA;
		}
	}

	virtual void writeBank1(u16 offset, u8 data) {
		if (offset & 0x2000) {
			if (hw.has(hw.RUMBLE)) {
				rumbleEnabled = (data & 0x8);
				data &= 0x7; // ignore bit 3 for ram bank when it's being used for rumble
			}

			if (hw.has(hw.RAM)) {
				ramBank = (data & 0xF);
			}
		}
	}
};