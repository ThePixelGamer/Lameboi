#pragma once

#include "SoundControl.h"
#include "Util/Types.h"

class Square {
protected:
	SoundControl& control;
	bool& soundOn;

	// internal
	u16 frequencyTimer = 0;
	u8 sequence = 0;
	u8 lengthCounter = 0;
	bool runEnvelope = false;
	int envelopeTimer = 0;
	s32 volume = 0;

	// registers
	u8 soundLength = 0;
	u8 waveDuty = 0;
	u8 envelopeSweep = 0;
	bool envelopeIncrease = false;
	u8 initialVolume = 0;
	u16 frequency = 0;
	bool lengthEnable = false;

	short output = 0;

	Square(SoundControl& control, bool& soundOn) : control(control), soundOn(soundOn) {}

public:
	Square(SoundControl& control) : Square(control, control.sound2On) {
		reset();
	}

	void update();
	virtual void trigger();
	void reset();

	void envelope();
	void lengthControl();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	float sample() {
		return static_cast<float>(output);
	}

	void resetWaveDuty() {
		sequence = 0;
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency) * 2;
	}

	void reloadEnvTimer() {
		envelopeTimer = envelopeSweep;
		if (envelopeTimer == 0)
			envelopeTimer = 8;
	}
};