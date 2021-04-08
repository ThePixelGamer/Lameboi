#include "Noise.h"

#include <algorithm>
#include <iostream>

void Noise::update() {
	if (frequencyTimer > 0 && --frequencyTimer == 0) {
		reloadFrequency();

		u8 xorBit = lfsr & 0x1;
		lfsr >>= 1;
		xorBit ^= (lfsr & 0x1) ^ 1;

		lfsr |= (xorBit << 14);

		if (lfsrWidth) {
			lfsr &= 0xFFBF; // clear bit 6
			lfsr |= (xorBit << 6);
		}

		output = (lfsr & 0x1) * (envelope.volume & 0xF);
	}
}

void Noise::trigger() {
	if (dacOn) {
		soundOn = true;
	}

	if (length.counter == 0) {
		length.counter = 64;
	}

	envelope.reload();

	reloadFrequency();
	lfsr = 0;
}

void Noise::reset() {
	Channel::reset();
	envelope.reset();

	lfsr = 0;
	frequencyTimer = 0;
	dacOn = false;

	divisorCode = 0;
	lfsrWidth = false;
	clockShift = 0;
}

u8 Noise::read(u8 reg) {
	switch (reg) {
		case 0x20: return 0xFF;
		case 0x21: return envelope.read();
		case 0x22: return (clockShift << 4) | (lfsrWidth << 3) | (divisorCode);
		case 0x23: return 0x80 | (length.enable << 6) | 0x3F;

		default:
			std::cout << "Reading from unknown Noise register: NRx" << +reg << std::endl;
			return 0xFF;
	}
}

void Noise::write(u8 reg, u8 value) {
	switch (reg) {
		case 0x20:
			length.counter = 64 - (value & 0x3F);
			break;

		case 0x21:
			envelope.write(value);

			dacOn = (value & 0xF8);
			if (!dacOn) {
				soundOn = false;
			}
			break;

		case 0x22:
			divisorCode = (value & 0x7);
			lfsrWidth = (value & 0x8);
			clockShift = (value >> 4);
			break;

		case 0x23:
			length.enable = (value & 0x40);

			if (value & 0x80) {
				trigger();
			}
			break;

		default:
			std::cout << "Writing to unknown Noise register: NRx" << +reg << std::endl;
			break;
	}
}
