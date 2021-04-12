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

		if (timer && --timer == 0) {
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

	bool dacOn() {
		return initialVolume || increase;
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
		bool newIncrease = (value & 0x8);

		// zombie mode 
		// todo: verify this as sameboy seems to implement something completely different
		// https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Obscure_Behavior
		if (sweep == 0 && run) {
			volume++;
		}
		else if (!increase) {
			volume += 2;
		}

		if (increase != newIncrease) {
			volume = 16 - volume;
		}

		volume &= 0xF;

		// regs write
		sweep = (value & 0x7);
		increase = newIncrease;
		initialVolume = (value >> 4);
	}
};