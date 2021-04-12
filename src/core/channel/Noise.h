#pragma once

#include "Channel.h"
#include "Envelope.h"
#include "util/Types.h"

class Noise : public Channel {
public:
	Envelope envelope;

private:
	// internal
	u16 lfsr;
	u16 frequencyTimer;
	bool dacOn;

	// registers
	u8 divisorCode;
	bool lfsrWidth;
	u8 clockShift;

public:
	Noise(bool& controlPower, const u8& sequencerStep) : Channel(controlPower, sequencerStep) {
		reset();
	}

	void update();
	void trigger();
	void reset();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

private:
	void reloadFrequency() {
		u8 divisor = (divisorCode == 0) ? 8 : (divisorCode * 16);
		frequencyTimer = divisor << clockShift;
		frequencyTimer /= 2;
	}
};