#include "Square.h"

#include <algorithm>
#include <iostream>

constexpr short dutyTable[4][8] = {
	{ -1, -1, -1, -1, -1, -1, -1, +1 },
	{ +1, -1, -1, -1, -1, -1, -1, +1 },
	{ +1, -1, -1, -1, -1, +1, +1, +1 },
	{ -1, +1, +1, +1, +1, +1, +1, -1 }
};

void Square::update() {
	if (--frequencyTimer == 0) {
		reloadFrequency();
		if (++sequence == 8)
			sequence = 0;
	}

	if (soundOn) {
		output = dutyTable[waveDuty][sequence] * (volume & 0xF);
	}
	else {
		output = 0;
	}
}

void Square::trigger() {
	soundOn = true;

	if (lengthCounter == 0) {
		lengthCounter = 64;
	}

	reloadFrequency();

	reloadEnvTimer();
	runEnvelope = true;

	volume = initialVolume;
}

void Square::reset() {
	soundLength = 0;
	waveDuty = 0;

	envelopeSweep = 0;
	envelopeIncrease = false;
	initialVolume = 0;
	volume = 0;

	frequency = 0;
	lengthEnable = false;
}

void Square::envelope() {
	if (--envelopeTimer > 0)
		return;
	
	reloadEnvTimer();

	if (runEnvelope && envelopeTimer != 8) {
		volume += (envelopeIncrease) ? 1 : -1;

		volume = std::clamp(volume, 0, 15);

		if (volume == 0 || volume == 15) {
			runEnvelope = false;
		}
	}
}

void Square::lengthControl() {
	if (lengthCounter != 0 && lengthEnable) {
		if (--lengthCounter == 0) {
			soundOn = false;
		}
	}
}

u8 Square::read(u8 reg) {
	switch (reg) {
		case 0x11: case 0x16: return (waveDuty << 6) | 0x3F;
		case 0x12: case 0x17: return (initialVolume << 4) | (envelopeIncrease << 3) | (envelopeSweep);
		case 0x13: case 0x18: return 0xFF;
		case 0x14: case 0x19: return 0x80 | (lengthEnable << 6) | 0x7;

		default:
			std::cout << "Reading from unknown Square register: NRx" << +reg << std::endl;
			return 0xFF;
	}
}

void Square::write(u8 reg, u8 value) {
	if (!control.soundOn && (reg != 0x11 && reg != 0x16)) {
		return;
	}

	switch (reg) {
		case 0x11: case 0x16:
			lengthCounter = 64 - (value & 0x3F);
			waveDuty = (value >> 6);
			break;

		case 0x12: case 0x17:
			envelopeSweep = (value & 0x7);
			envelopeIncrease = (value & 0x8);
			initialVolume = (value >> 4);
			break;

		case 0x13: case 0x18:
			frequency &= 0x700;
			frequency |= value;
			break;

		case 0x14: case 0x19:
			frequency &= 0xFF;
			frequency |= (value & 0x7) << 8;
			lengthEnable = (value & 0x40);

			if (value & 0x80) {
				trigger();
			}
			break;

		default:
			std::cout << "Writing to unknown Square register: NRx" << +reg << std::endl;
			break;
	}
}
