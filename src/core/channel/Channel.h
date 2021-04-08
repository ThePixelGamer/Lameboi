#pragma once

#include "LengthCounter.h"
#include "SoundControl.h"

class Channel {
public:
	LengthCounter length;

protected:
	SoundControl& control;
	bool& soundOn;

	u8 output; // 0 - 15

	Channel(SoundControl& control, bool& soundOn) : control(control), length(soundOn), soundOn(soundOn) {
		reset();
	}
	
	void reset() {
		length.reset();

		output = 0;
	}

public:
	float sample() const {
		if (!soundOn) {
			return 0.0f;
		}

		return static_cast<float>(15 - (output * 2)) / 15.0f;
	}
};