#pragma once

#include "MBC.h"

class MBC1 : public MBC {
	bool altBankMode = false;

public:
	MBC1(Cartridge& hw) : MBC(hw) {}

	// todo: rewrite, altBankMode is implemented incorrectly
	virtual void writeBank0(u16 offset, u8 data) {
		if (offset & 0x2000) {
			romBank &= 0x60;
			u8 bank = (data & 0x1F);
			romBank |= (bank == 0) ? 1 : bank;
		}
		else {
			ramEnabled = (data & 0xF) == 0xA;
		}
	}

	virtual void writeBank1(u16 offset, u8 data) {
		if (offset & 0x2000) {
			altBankMode = (data & 0x1);
		}
		else {
			if (altBankMode) {
				if (hw.getHeader()->getMaxRamBanks() >= 4) {
					ramBank = (data & 0x3);
				}
				else if (hw.getHeader()->getMaxRomBanks() >= 64) {
					romBank &= 0x1F;
					romBank |= (data & 0x3) << 5;
				}
			}
			else {
				ramBank = 0;
				romBank = (data & 0x7F);
			}
		}
	};

	virtual u8 readBank0(u16 offset) {
		return hw.getRomBank((altBankMode) ? (romBank & 0x60) : 0)[offset];
	}
};