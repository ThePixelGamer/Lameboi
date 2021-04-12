#include "Wave.h"

#include <algorithm>
#include <iostream>

void Wave::update() {
	if (frequencyTimer && --frequencyTimer == 0) {
		reloadFrequency();
		if (++wavePosition == 32)
			wavePosition = 0;
	}

	u8 wave = wavePattern[wavePosition >> 1];
	if ((wavePosition & 1) == 0) {
		wave &= 0xF;
	}
	else {
		wave >>= 4;
	}

	output = wave >> (volumeCode - 1);
}

void Wave::trigger() {
	if (dacOn) {
		soundOn = true;
	}

	if (length.counter == 0) {
		length.counter = 256;
		length.enable = false;
	}

	reloadFrequency();
	wavePosition = 0;
}

void Wave::reset() {
	Channel::reset();

	volumeCode = 0;
	frequency = 0;
	dacOn = false;
}

u8 Wave::read(u8 reg) {
	switch (reg) {
		case 0x1A: return (dacOn << 7) | 0x7F;
		case 0x1B: return 0xFF;
		case 0x1C: return (volumeCode << 5) | 0x9F;
		case 0x1D: return 0xFF;
		case 0x1E: return 0x80 | (length.enable << 6) | 0x3F;

		default:
			std::cout << "Reading from unknown Wave register: NRx" << +reg << std::endl;
			return 0xFF;
	}
}

void Wave::write(u8 reg, u8 value) {
	if (!controlPower && reg != 0x1B) {
		return;
	}

	switch (reg) {
		case 0x1A:
			dacOn = (value & 0x80);
			if (!dacOn) {
				soundOn = false;
			}
			break;

		case 0x1B:
			length.counter = 256 - value;
			break;

		case 0x1C:
			volumeCode = (value & 0x60) >> 5;
			break;

		case 0x1D:
			frequency &= 0x700;
			frequency |= value;
			break;

		case 0x1E:
			frequency &= 0xFF;
			frequency |= (value & 0x7) << 8;
			length.enable = (value & 0x40);

			if (value & 0x80) {
				trigger();
			}
			break;

		default:
			std::cout << "Writing to unknown Wave register: NRx" << +reg << std::endl;
			break;
	}
}

u8 Wave::readPattern(u8 offset) {
	return wavePattern[offset & 0xF];
}

void Wave::writePattern(u8 offset, u8 value) {
	wavePattern[offset & 0xF] = value;
}
