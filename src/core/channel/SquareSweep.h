#pragma once

#include "Square.h"
#include "util/Types.h"

class SquareSweep : public Square {
	// internal
	bool sweepEnable;
	u16 shadowFrequency;
	int sweepTimer;

	// registers
	u8 sweepShifts;
	bool sweepDecrease;
	u8 sweepTimerLoad;

public:
	SquareSweep(bool& controlPower, const u8& sequencerStep) : Square(controlPower, sequencerStep) {
		reset();
	}

	virtual void trigger() override {
		Square::trigger();

		shadowFrequency = frequency;
		reloadSweepTimer();

		sweepEnable = sweepTimer > 0 || sweepShifts > 0;
		if (sweepShifts != 0) {
			overflowCheck(calcFrequency());
		}
	}

	void reset() {
		Square::reset();

		sweepEnable = false;
		shadowFrequency = 0;
		sweepTimer = 0;

		sweepShifts = 0;
		sweepDecrease = false;
		sweepTimerLoad = 0;
	}

	u8 read(u8 reg) {
		switch (reg) {
			case 0: return 0x80 | (sweepTimerLoad << 4) | (sweepDecrease << 3) | (sweepShifts);
			default: return Square::read(reg);
		}
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
			case 0: 
				if (controlPower) {
					sweepShifts = (value & 0x7);
					sweepDecrease = (value & 0x8);
					sweepTimerLoad = (value & 0x70) >> 4;
				}
				break;

			default: Square::write(reg, value); break;
		}
	}

	void sweep() {
		if (--sweepTimer <= 0) {
			reloadSweepTimer();

			if (sweepEnable && sweepTimerLoad != 0) {
				u16 adjustedFrequency = calcFrequency();
				if (overflowCheck(adjustedFrequency) && sweepShifts != 0) {
					frequency = shadowFrequency = adjustedFrequency;
					overflowCheck(calcFrequency());
				}
			}
		}
	}

private:
	u16 calcFrequency() {
		s16 adjustedFrequency = shadowFrequency >> sweepShifts;

		if (sweepDecrease) {
			adjustedFrequency = -adjustedFrequency;
		}

		return shadowFrequency + adjustedFrequency;
	}

	bool overflowCheck(u16 freq){
		if (freq > 2047) {
			soundOn = false;
			return false;
		}

		return true;
	}

	void reloadSweepTimer() {
		sweepTimer = sweepTimerLoad;
		if (sweepTimer == 0)
			sweepTimer = 8;
	}
};
