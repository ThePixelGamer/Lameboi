#pragma once

#include <iostream>

#include "util/Types.h"

class APU;

class SoundControl {
	APU& apu;

public:
	struct { //FF24 Channel control / ON-OFF / Volume
		u8 SO1Volume : 3; // right headphone
		u8 vinToS01 : 1;
		u8 SO2Volume : 3; // left headphone
		u8 vinToS02 : 1;
	} NR50;

	struct { //FF25 Selection of Sound output terminal
		u8 sound1ToSO1 : 1;
		u8 sound2ToSO1 : 1;
		u8 sound3ToSO1 : 1;
		u8 sound4ToSO1 : 1;
		u8 sound1ToSO2 : 1;
		u8 sound2ToSO2 : 1;
		u8 sound3ToSO2 : 1;
		u8 sound4ToSO2 : 1;
	} NR51;

	bool sound1On;
	bool sound2On;
	bool sound3On;
	bool sound4On;
	bool soundOn;

	SoundControl(APU& apu) : apu(apu) {
		reset();

		sound1On = 0;
		sound2On = 0;
		sound3On = 0;
		sound4On = 0;
		soundOn = 0;
	}
	
	void reset();
	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};