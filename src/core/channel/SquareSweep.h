#pragma once

#include "Square.h"
#include "SoundControl.h"
#include "util/Types.h"

class SquareSweep : public Square {
	// internal
	u16 shadowFrequency = 0;
	int sweepTimer = 0;

	// registers
	u8 sweepShifts = 0;
	bool sweepDecrease = false;
	u8 sweepTime = 0; // n/128hz

public:
	SquareSweep(SoundControl& control) : Square(control, control.sound1On) {
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