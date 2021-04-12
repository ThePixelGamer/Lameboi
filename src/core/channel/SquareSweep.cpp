#include "SquareSweep.h"

#include <algorithm>
#include <iostream>

void SquareSweep::trigger() {
	Square::trigger();

	shadowFrequency = frequency;
	reloadSweepTimer();

	if (sweepShifts != 0) {
		overflowCheck(calcFrequency());
	}
}

void SquareSweep::reset() {
	Square::reset();

	shadowFrequency = 0;
	sweepTimer = 0;

	sweepShifts = 0;
	sweepDecrease = false;
	sweepTime = 0;
}

void SquareSweep::sweep() {
	if (sweepTimer && --sweepTimer <= 0) {
		reloadSweepTimer();

		if (sweepTime != 0) {
			u16 adjustedFrequency = calcFrequency();
			if (overflowCheck(adjustedFrequency) && sweepShifts != 0) {
				frequency = shadowFrequency = adjustedFrequency;
				overflowCheck(calcFrequency());
			}
		}
	}
}

u16 SquareSweep::calcFrequency() {
	s16 adjustedFrequency = shadowFrequency >> sweepShifts;

	if (sweepDecrease) {
		adjustedFrequency = -adjustedFrequency;
	}

	return shadowFrequency + adjustedFrequency;
}

bool SquareSweep::overflowCheck(u16 freq) {
	if (freq > 2047) {
		soundOn = false;
		return false;
	}

	return true;
}

u8 SquareSweep::read(u8 reg) {
	if (reg == 0x10) {
		return 0x80 | (sweepTime << 4) | (sweepDecrease << 3) | (sweepShifts);
	}
	else {
		return Square::read(reg);
	}
}

void SquareSweep::write(u8 reg, u8 value) {
	if (!controlPower && reg != 0x11) {
		return;
	}

	if (reg == 0x10) {
		sweepShifts = (value & 0x7);
		sweepDecrease = (value & 0x8);
		sweepTime = (value & 0x70) >> 4;
	}
	else {
		_write(reg, value);
	}
}