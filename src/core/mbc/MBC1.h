#pragma once

#include "MBC.h"

#include <array>

class MBC1 : public MBC {
	bool ramEnabled = false;
	bool altBankMode = false;
	u8 romBank = 1; // 1h-7Fh
	u8 ramBank = 0; // 0h-3h = banks 

	std::array<std::array<u8, 0x4000>, 0x80> romBanks; // 00h-7Fh
	std::array<std::array<u8, 0x2000>, 0x4> ramBanks; // 00h-03h

public:
	MBC1(const std::string& romPath_, bool ram = false, bool battery = false) : MBC(romPath_, ram, battery) {
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
		else if (location >= 0x6000) {
			altBankMode = (data & 0x1);
		}
		else if (location >= 0x4000) {
			if (altBankMode) {
				if (_getMaxRamBanks(romBanks[0][0x149]) >= 4) {
					ramBank = (data & 0x3);
				}
				else if (_getMaxRomBanks(romBanks[0][0x148]) >= 64) {
					romBank &= 0x1F;
					romBank |= (data & 0x3) << 5;
				}
			}
			else {
				ramBank = 0;
				romBank = (data & 0x7F);
			}
		}
		else if (location >= 0x2000) {
			romBank &= 0x60;
			u8 bank = (data & 0x1F);
			romBank |= (bank == 0) ? 1 : bank;
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
			return romBanks[(altBankMode) ? (romBank & 0x60) : 0][location];
		}

		return 0xFF;
	}
};