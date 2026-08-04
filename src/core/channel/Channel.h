#pragma once

#include "util/Log.h"
#include "util/Types.h"

struct LengthCounter {
private:
	bool& soundOn;

public:
	u16 size = 64;
	u16 counter;
	bool enable;

	LengthCounter(bool& soundOn) : soundOn(soundOn) {
		reset();
	}

	void reset() {
		counter = 0;
		enable = false;
	}

	void tick() {
		if (enable && counter) {
			if (--counter == 0) {
				soundOn = false;
			}
		}
	}

	u8 read() { return size - 1; }
	void write(u8 v) { counter = size - (v & (size - 1)); }
};

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
		return (soundOn) ? float(15 - (output * 2)) / 15.0f : 0.0f;
	}

	float getL() {
		return (left) ? sample() : 0.0f;
	}

	float getR() {
		return (right) ? sample() : 0.0f;
	}
};
