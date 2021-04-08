#pragma once

#include "util/Types.h"

struct Envelope {
	// interal
	bool run;
	s32 timer;
	u8 volume;

	// registers
	u8 sweep;
	bool increase;
	u8 initialVolume;

	Envelope() {
		reset();
	}

	void reset() {
		run = false;
		timer = 8;
		volume = 0;

		sweep = 0;
		increase = false;
		initialVolume = 0;
	}

	void tick() {
		if (!run) {
			return;
		}

		if (--timer <= 0) {
			reloadTimer();

			if (sweep == 0) {
				return;
			}

			if (increase) {
				if (volume != 15) {
					++volume;
				}
				else {
					run = false;
				}
			}
			else {
				if (volume != 0) {
					--volume;
				}
				else {
					run = false;
				}
			}
		}
	}

	void reloadTimer() {
		timer = sweep;
		if (timer == 0)
			timer = 8;
	}

	void reload() {
		reloadTimer();
		run = true;
		volume = initialVolume;
	}

	u8 read() {
		return (initialVolume << 4) | (increase << 3) | (sweep);
	}

	void write(u8 value) {
		sweep = (value & 0x7);
		increase = (value & 0x8);
		initialVolume = (value >> 4);
	}
};