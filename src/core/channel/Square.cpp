#include "Square.h"

#include <algorithm>
#include <iostream>

constexpr u8 dutyTable[4][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 }
};

void Square::update() {
	if (frequencyTimer && --frequencyTimer == 0) {
		reloadFrequency();
		if (++sequence == 8)
			sequence = 0;
	}

	output = dutyTable[waveDuty][sequence] * (envelope.volume & 0xF);
}

void Square::trigger() {
	if (dacOn) {
		soundOn = true;
	}

	if (length.counter == 0) {
		length.counter = 64;
		length.enable = false;
	}

	reloadFrequency();

	envelope.reload();
}

void Square::reset() {
	Channel::reset();
	envelope.reset();

	frequencyTimer = 0;
	sequence = 0;
	dacOn = false;

	waveDuty = 0;
	frequency = 0;
}

u8 Square::read(u8 reg) {
	switch (reg) {
		case 0x11: case 0x16: return (waveDuty << 6) | 0x3F;
		case 0x12: case 0x17: return envelope.read();
		case 0x13: case 0x18: return 0xFF;
		case 0x14: case 0x19: return 0x80 | (length.enable << 6) | 0x3F;

		default:
			std::cout << "Reading from unknown Square register: NRx" << +reg << std::endl;
			return 0xFF;
	}
}

void Square::write(u8 reg, u8 value) {
	if (!controlPower && reg != 0x16) {
		return;
	}

	_write(reg, value);
}

void Square::_write(u8 reg, u8 value) {
	switch (reg) {
		case 0x11: case 0x16:
			length.counter = 64 - (value & 0x3F);
			
			if (controlPower)
				waveDuty = (value >> 6);
			break;

		case 0x12: case 0x17:
			envelope.write(value);

			dacOn = (value & 0xF8);
			if (!dacOn) {
				soundOn = false;
			}
			break;

		case 0x13: case 0x18:
			frequency &= 0x700;
			frequency |= value;
			break;

		case 0x14: case 0x19:
			frequency &= 0xFF;
			frequency |= (value & 0x7) << 8;
			length.enable = (value & 0x40);

			if (value & 0x80) {
				trigger();
			}
			break;

		default:
			std::cout << "Writing to unknown Square register: NRx" << +reg << std::endl;
			break;
	}
}
