#pragma once

#include "Channel.h"
#include "Envelope.h"
#include "LengthCounter.h"
#include "util/Types.h"

class Square : public Channel {
public:
	Envelope envelope;

	// internal
	u16 frequencyTimer;
	u8 sequence;
	bool dacOn;

	// registers
	u8 waveDuty;
	u8 initialVolume;
	u16 frequency;

protected:
	Square(SoundControl& control, bool& soundOn) : Channel(control, soundOn) {
		reset();
	}

public:
	Square(SoundControl& control) : Square(control, control.sound2On) {}

	void update();
	virtual void trigger();
	void reset();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	void resetWaveDuty() {
		sequence = 0;
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency) * 2;
	}
};