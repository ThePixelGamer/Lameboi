#pragma once

#include "MBC.h"
#include "Util/Common.h"

#include <iostream>

class MBC5 : public MBC {
	bool rumble = false;
	bool ramEnabled = false;
	bool rumbleEnabled = false;
	u16 romBank = 1; // 0h-1FFh
	u8 ramBank = 0; // 0h-Fh = banks

	std::array<std::array<u8, 0x4000>, 0x200> romBanks; // 00h-1FFh
	std::array<std::array<u8, 0x2000>, 0x10> ramBanks; // 00h-0Fh

public:
	MBC5(bool ram_ = false, bool battery_ = false, bool rumble_ = false) : MBC(ram_, battery_), rumble(rumble_) {
		romBanks.fill({});
		ramBanks.fill({});
	}


	virtual void setup(std::istream& stream) {
		stream.read((char*)romBanks[0].data(), 0x4000);

		u16 maxBanks = _getMaxRomBanks(romBanks[0][0x148]);

		// 01h - Amount of Banks
		for (u8 bankAmount = 1; bankAmount < maxBanks; ++bankAmount) {
			stream.read((char*)romBanks[bankAmount].data(), 0x4000);
		}

		//probably change this to only save the ram banks used
		if (battery) {
			std::ifstream file(_getName(&romBanks[0][0x134]), std::ios_base::binary);
			maxBanks = _getMaxRamBanks(romBanks[0][0x149]);

			for (size_t i = 0; i < maxBanks; ++i) {
				file.read((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void save() {
		// dump ram to file
		if (battery) {
			std::ofstream file(_getName(&romBanks[0][0x134]), std::ios_base::binary);
			u16 maxBanks = _getMaxRamBanks(romBanks[0][0x149]);

			for (size_t i = 0; i < maxBanks; ++i) {
				file.write((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void close() {
		save();

		for (auto& bank : romBanks) {
			bank.fill(0xFF);
		}

		for (auto& bank : ramBanks) {
			bank.fill(0xFF);
		}
	}

	virtual void write(u16 location, u8 data) {
		if (location >= 0xA000) {
			if (ramEnabled && ram) {
				ramBanks[ramBank][location - 0xA000] = data;
			}
		}
		else if (location >= 0x6000) {}
		else if (location >= 0x4000) {
			if (rumble) {
				rumbleEnabled = (data & 0x8);
				data &= 0x7; // ignore bit 3 for ram bank when it's being used for rumble
			}

			if (ram) {
				ramBank = (data & 0xF);
			}
		}
		else if (location >= 0x3000) {
			romBank &= 0xFF;
			romBank |= (data & 0x1) << 8;
		}
		else if (location >= 0x2000) {
			romBank &= 0x100;
			romBank |= data;
		}
		else {
			ramEnabled = (data & 0xF) == 0xA;
		}
	}

	virtual u8 read(u16 location) {
		if (location >= 0xA000) {
			if (ramEnabled && ram) {
				return ramBanks[ramBank][location - 0xA000];
			}
		}
		else if (location >= 0x4000) {
			return romBanks[romBank][location - 0x4000];
		}
		else {
			return romBanks[0][location];
		}

		return 0xFF;
	}
};