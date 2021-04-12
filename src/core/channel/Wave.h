#pragma once

#include <array>

#include "Channel.h"
#include "util/Types.h"

class Wave : public Channel {
	//internal
	u16 frequencyTimer = 0;
	u8 wavePosition = 0;
	bool dacOn = false;
	
	// registers
	std::array<u8, 0x10> wavePattern{};
	u8 volumeCode = 0;
	u16 frequency = 0;

public:
	Wave(bool& controlPower, const u8& sequencerStep) : Channel(controlPower, sequencerStep) {
		reset();
	}

	void update();
	void trigger();
	void reset();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

	u8 readPattern(u8 offset);
	void writePattern(u8 offset, u8 value);

	void resetWaveBuffer() {
		wavePattern.fill(0);
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency);
	}
};