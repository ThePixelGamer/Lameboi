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

void Wave::write(u8 reg, u8 value) {
	if (!controlPower && reg != 0x1B) {
		return;
	}

	switch (reg) {
		case 0:
			dacOn = (value & 0x80);
			if (!dacOn) {
				soundOn = false;
			}
			break;

		case 1:
			length.counter = 256 - value;
			break;

		case 2:
			volumeCode = (value & 0x60) >> 5;
			break;

		case 3:
			frequency &= 0x700;
			frequency |= value;
			break;

		case 4:
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
