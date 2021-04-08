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
	Noise(SoundControl& control) : Channel(control, control.sound4On) {
		reset();
	}

	void update();
	void trigger();
	void reset();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

private:
	void reloadFrequency() {
		constexpr u16 divisors[] = { 8, 16, 32, 48, 64, 80, 96, 112 };
		frequencyTimer = divisors[divisorCode] << clockShift;
	}
};