#pragma once

#include "Channel.h"
#include "Envelope.h"
#include "util/Log.h"
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

public:
	Square(bool& power, const u8& step) : Channel(power, step) {
		reset();
	}

	void update()  {
		if (frequencyTimer && --frequencyTimer == 0) {
			reloadFrequency();
			if (++sequence == 8)
				sequence = 0;
		}

		constexpr u8 dutyTable[4][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1, 0 }
		};
		output = dutyTable[waveDuty][sequence] * (envelope.volume & 0xF);
	}

	virtual void trigger() {
		if (dacOn) {
			soundOn = true;
		}

		if (length.counter == 0) {
			length.counter = 64;
		}

		reloadFrequency();

		envelope.reload();
	}

	void reset()  {
		Channel::reset();
		envelope.reset();

		frequencyTimer = 0;
		sequence = 0;
		dacOn = false;

		waveDuty = 0;
		frequency = 0;
	}

	void resetWaveDuty() {
		sequence = 0;
	}

	u8 read(u8 reg) {
		switch (reg) {
			case 1: return (waveDuty << 6) | 0x3F;
			case 2: return envelope.read(); 
			case 3: return 0xFF;
			case 4: return 0x80 | (length.enable << 6) | 0x3F;

			default: 
				LB_ERROR(Audio, "Writing to unknown Square Register {:02X}", reg);
				return 0xFF;
		} 
	}

	void write(u8 reg, u8 v) {
		switch (reg) {
			case 1: 
				length.write(v);
				if (controlPower) waveDuty = (v >> 6);
				break;

			case 2: 
				if (controlPower) {
					envelope.write(v);
					dacOn = v & 0xF8;
					if (!dacOn) soundOn = false;
				}
				break;

			case 3: 
				if (controlPower) {
					frequency &= 0x700;
					frequency |= v;
				}
				break;

			case 4:
				if (controlPower) {
					frequency &= 0xFF;
					frequency |= (v & 0x7) << 8;
					length.enable = (v & 0x40);

					if (v & 0x80) trigger();
				}
				break;
			default: LB_ERROR(Audio, "Writing to unknown Square Register {:02X}", reg);
		} 
	}

private:
	void reloadFrequency() {
		frequencyTimer = (2048 - frequency);
	}
};
