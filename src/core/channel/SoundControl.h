#pragma once

#include <iostream>

#include "util/Types.h"

class APU;

class SoundControl {
	APU& apu;

public:
	// FF24 Channel control / ON-OFF / Volume
	u8 rightVolume; // right headphone
	bool vinRight;
	u8 leftVolume; // left headphone
	bool vinLeft;

	// FF25 Selection of Sound output terminal
	bool snd1Right;
	bool snd2Right;
	bool snd3Right;
	bool snd4Right;
	bool snd1Left;
	bool snd2Left;
	bool snd3Left;
	bool snd4Left;

	// FF26 Sound Enables
	bool sound1On;
	bool sound2On;
	bool sound3On;
	bool sound4On;
	bool soundOn;

	SoundControl(APU& apu) : apu(apu) {
		reset();
	}
	
	void reset();
	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};