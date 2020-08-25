#pragma once

#include "IMBC.h"

class MBC0 : public IMBC {
	std::array<u8, 0x4000> bank0; // 0x0000
	std::array<u8, 0x4000> bank1; // 0x8000
	std::array<u8, 0x2000> ERAM; // 0xA000

public:
	MBC0() : IMBC() {
		bank0.fill(0);
		bank1.fill(0);
		ERAM.fill(0);
	}

	virtual void setup(std::istream& stream) {
		stream.read((char*)bank0.data(), 0x4000);
		stream.read((char*)bank1.data(), 0x4000);
	}

	virtual void save() {}

	virtual void close() {
		bank0.fill(0xFF);
		bank1.fill(0xFF);
		ERAM.fill(0xFF);
	}

	virtual void write(u16 location, u8 data) {
		if (location >= 0xA000) {
			ERAM[location] = data;
		}
	}

	virtual u8 read(u16 location) {
		if (location >= 0xA000) {
			return ERAM[location - 0xA000];
		}
		else if (location >= 0x4000) { //maybe have this restricted to 4000h-7FFFh?
			return bank1[location - 0x4000];
		}
		else {
			return bank0[location];
		}
	}
};