#include "Wave.h"

#include <algorithm>
#include <iostream>

void Wave::update() {
	if (--frequencyTimer == 0) {
		reloadFrequency();
		if (++wavePosition == 32)
			wavePosition = 0;
	}

	if (soundOn && volumeCode != 0 && playSound) {
		u8 wave = wavePattern[wavePosition >> 1];
		if ((wavePosition & 1) == 0) {
			wave &= 0xF;
		}
		else {
			wave >>= 4;
		}

		output = wave >> (volumeCode - 1);
	}
	else {
		output = 0;
	}
}

void Wave::trigger() {
	soundOn = true;

	if (lengthCounter == 0) {
		lengthCounter = 256;
	}

	reloadFrequency();
	wavePosition = 0;
}

void Wave::reset() {
	playSound = false;
	soundLength = 0;
	volumeCode = 0;
	frequency = 0;
	lengthEnable = false;
}

void Wave::lengthControl() {
	if (lengthCounter != 0 && lengthEnable) {
		if (--lengthCounter == 0) {
			soundOn = false;
		}
	}
}

u8 Wave::read(u8 reg) {
	switch (reg) {
		case 0x1A: return (playSound << 7) | 0x7F;
		case 0x1B: return 0xFF;
		case 0x1C: return (volumeCode << 5) | 0x9F;
		case 0x1D: return 0xFF;
		case 0x1E: return 0x80 | (lengthEnable << 6) | 0x7;

		default:
			std::cout << "Reading from unknown Wave register: NRx" << +reg << std::endl;
			return 0xFF;
	}
}

void Wave::write(u8 reg, u8 value) {
	switch (reg) {
		case 0x1A:
			playSound = (value & 0x80);
			break;

		case 0x1B:
			lengthCounter = 256 - value;
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
			lengthEnable = (value & 0x40);

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
