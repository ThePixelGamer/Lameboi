#include "SquareSweep.h"

#include <algorithm>
#include <iostream>

void SquareSweep::trigger() {
	Square::trigger();

	shadowFrequency = frequency;
	reloadSweepTimer();
	runSweep = (sweepTime != 0);

	if (sweepShifts != 0) {
		runSweep = true;
		_sweep();
	}
}

void SquareSweep::reset() {
	Square::reset();

	sweepShifts = 0;
	sweepDecrease = false;
	sweepTime = 0;
}

void SquareSweep::sweep() {
	if (--sweepTimer > 0)
		return;

	reloadSweepTimer();

	_sweep();
}

void SquareSweep::_sweep() {
	auto frequencyCalc = [this]() {
		u16 adjustedFrequency = shadowFrequency >> sweepShifts;

		if (sweepDecrease) {
			adjustedFrequency = -adjustedFrequency;
		}

		adjustedFrequency += shadowFrequency;

		return adjustedFrequency;
	};

	auto overflowCheck = [this](u16 freq) {
		if (freq > 2047) {
			soundOn = false;
			return true;
		}
		return false;
	};

	if (runSweep && sweepTimer != 8) {
		u16 adjustedFrequency = frequencyCalc();

		if (!overflowCheck(adjustedFrequency) && sweepShifts != 0) {
			frequency = shadowFrequency = adjustedFrequency;

			overflowCheck(frequencyCalc());
		}
	}
}

u8 SquareSweep::read(u8 reg) {
	if (reg == 0x0) {
		return 0x80 | (sweepTime << 4) | (sweepDecrease << 3) | (sweepShifts);
	}
	else {
		return Square::read(reg);
	}
}

void SquareSweep::write(u8 reg, u8 value) {
	// need to change to allow for writes to the length counter
	if (!control.soundOn) {
		return;
	}

	if (reg == 0x0) {
		sweepShifts = (value);
		sweepDecrease = (value & 0x8);
		sweepTime = (value >> 4);
	}
	else {
		Square::write(reg, value);
	}
}