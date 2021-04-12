#pragma once

#include "Square.h"
#include "util/Types.h"

class SquareSweep : public Square {
	// internal
	u16 shadowFrequency;
	int sweepTimer;

	// registers
	u8 sweepShifts;
	bool sweepDecrease;
	u8 sweepTime;

public:
	SquareSweep(bool& controlPower, const u8& sequencerStep) : Square(controlPower, sequencerStep) {
		reset();
	}

	virtual void trigger() override;
	void reset();
	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	void sweep();

private:
	u16 calcFrequency();
	bool overflowCheck(u16 freq);

	void reloadSweepTimer() {
		sweepTimer = sweepTime;
		if (sweepTimer == 0)
			sweepTimer = 8;
	}
};