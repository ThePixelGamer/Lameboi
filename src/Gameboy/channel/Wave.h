#pragma once

#include <array>

#include "SoundControl.h"
#include "Util/Types.h"

class Wave {
	SoundControl& control;
	bool& soundOn;

	u16 frequencyTimer = 0;
	u8 wavePosition = 0;
	u16 lengthCounter = 0;
	bool runEnvelope = false;
	int envelopeTimer = 0;

	// registers
	std::array<u8, 0x10> wavePattern{};
	bool playSound = false;
	u8 soundLength = 0;
	u8 volumeCode = 0;
	u16 frequency = 0;
	bool lengthEnable = false;

	short output = 0;

public:
	Wave(SoundControl& control) : control(control), soundOn(control.sound3On) {}

	void update();
	void trigger();
	void reset();

	void lengthControl();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	float sample() {
		return static_cast<float>(output);
	}

	void resetWaveBuffer() {
		wavePattern.fill(0);
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency) * 2;
	}
};