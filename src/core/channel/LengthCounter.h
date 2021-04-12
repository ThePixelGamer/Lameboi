#pragma once

#include "util/Types.h"

#include <algorithm>

struct LengthCounter {
private:
	bool& soundOn;

public:
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
		if (enable) {
			if (counter && --counter == 0) {
				soundOn = false;
			}
		}
	}
};