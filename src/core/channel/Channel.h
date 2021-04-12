#pragma once

#include "LengthCounter.h"

class Channel {
public:
	LengthCounter length;
	bool left, right;
	bool soundOn;

protected:
	bool& controlPower;
	const u8& sequencerStep;
	u8 output; // 0 - 15

	Channel(bool& controlPower, const u8& sequencerStep) : length(soundOn), controlPower(controlPower), sequencerStep(sequencerStep) {
		reset();
	}
	
	void reset() {
		length.reset();

		left = false;
		right = false;
		soundOn = false;
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