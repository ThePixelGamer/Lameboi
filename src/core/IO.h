#pragma once

#include "util/Common.h"
#include "util/Types.h"

class Gameboy;

class IO {
	Gameboy& gb;

public:
	IO(Gameboy& gb) : gb(gb) {}

	bool unused(u8 reg);

	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};