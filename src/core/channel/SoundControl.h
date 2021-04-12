#pragma once

#include <iostream>

#include "SquareSweep.h"
#include "Square.h"
#include "Wave.h"
#include "Noise.h"
#include "util/Types.h"

class APU;

class SoundControl {
public:
	// FF24 Channel control / ON-OFF / Volume
	u8 rightVolume; // right headphone
	bool vinRight;
	u8 leftVolume; // left headphone
	bool vinLeft;

	SquareSweep squareSweep;
	Square square;
	Wave wave;
	Noise noise;

	bool soundOn;
	u8 sequencerStep;

	// UI
	bool channel1On;
	bool channel2On;
	bool channel3On;
	bool channel4On;

	SoundControl() : squareSweep(soundOn, sequencerStep), square(soundOn, sequencerStep), wave(soundOn, sequencerStep), noise(soundOn, sequencerStep) {
		reset();
	}
	
	void reset();
	void resetRegs();

	float getOutput(bool right);
	
	void sequence();
	void step();
	
	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};