#pragma once

#include "util/Common.h"
#include "util/Types.h"

class Gameboy;

class IO {
	Gameboy& gb;

public:
	enum Register {
		JOYP = 0x0,
		SB = 0x01,
		SC = 0x02,
		DIV = 0x04,
		TIMA = 0x05,
		IF = 0x0F,
		DMA = 0x46,

		// HRAM 0x80 - 0xFE 
		IE = 0xFF
	};

	IO(Gameboy& gb) : gb(gb) {}

	bool unused(u8 reg);

	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};