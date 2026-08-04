#pragma once

#include <array>

#include "Channel.h"

class Wave : public Channel {
	//internal
	u16 frequencyTimer = 0;
	u8 wavePosition = 0;
	bool dacOn = false;
	
	// registers
	// 32 4-bit samples
	std::array<u8, 0x10> wavePattern{};
	u8 volumeCode = 0;
	u16 frequency = 0;

public:
	Wave(bool& controlPower, const u8& sequencerStep) : Channel(controlPower, sequencerStep) {
		length.size = 256;
		reset();
	}

	void update();
	void trigger();
	void reset();

	u8 read(u8 reg) {
		switch (reg) {
			case 0: return (dacOn << 7) | 0x7F;
			case 1: return 0xFF;
			case 2: return (volumeCode << 5) | 0x9F;
			case 3: return 0xFF;
			case 4: return 0x80 | (length.enable << 6) | 0x3F;

			default: 
				LB_INFO(Audio, "Reading from unknown Wave register: NR3{}", +reg);
				return 0xFF;
		}
	}
	void write(u8 reg, u8 value);

	u8 readPattern(u8 offset) {
		return wavePattern[offset & 0xF];
	}

	void writePattern(u8 offset, u8 value) {
		wavePattern[offset & 0xF] = value;
	}

	void resetWaveBuffer() {
		wavePattern.fill(0);
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency) * 2;
	}
};
