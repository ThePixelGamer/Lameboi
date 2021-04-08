#include "SoundControl.h"

#include "../APU.h"

void SoundControl::reset() {
	rightVolume = 0;
	vinRight = 0;
	leftVolume = 0;
	vinLeft = 0;

	snd1Right = 0;
	snd2Right = 0;
	snd3Right = 0;
	snd4Right = 0;
	snd1Left = 0;
	snd2Left = 0;
	snd3Left = 0;
	snd4Left = 0;

	sound1On = 0;
	sound2On = 0;
	sound3On = 0;
	sound4On = 0;
	soundOn = 0;
}

u8 SoundControl::read(u8 reg) {
	switch (reg) {
		case 0x24: // NR50
			return (vinLeft << 7) | (leftVolume << 4) | (vinRight << 3) | (rightVolume);

		case 0x25:
			return u8(snd1Right) | (snd2Right << 1) | (snd3Right << 2) | (snd4Right << 3) |
				(snd1Left << 4) | (snd2Left << 5) | (snd3Left << 6) | (snd4Left << 7);

		case 0x26: // NR52
			return (soundOn << 7) | 0x70 | (sound4On << 3) | (sound3On << 2) | (sound2On << 1) | u8(sound1On);

		default:
			std::cout << "Reading from unknown SoundControl register: NR5" << +reg << std::endl;
			return 0xFF;
	}
}

void SoundControl::write(u8 reg, u8 value) {
	switch (reg) {
		case 0x24: // NR50
			rightVolume = (value & 0x7);
			vinRight = (value & 0x8);
			leftVolume = ((value >> 4) & 0x7);
			vinLeft = (value & 0x80);
			break;

		case 0x25:
			snd1Right = (value & 0x1);
			snd2Right = (value & 0x2);
			snd3Right = (value & 0x4);
			snd4Right = (value & 0x8);
			snd1Left = (value & 0x10);
			snd2Left = (value & 0x20);
			snd3Left = (value & 0x40);
			snd4Left = (value & 0x80);
			break;

		case 0x26: // NR52
			soundOn = (value & 0x80);
			if (soundOn) {
				apu.sequencer = 0;
				apu.squareSweep.resetWaveDuty();
				apu.square.resetWaveDuty();
				apu.wave.resetWaveBuffer();
			}
			else {
				reset();
				apu.squareSweep.reset();
				apu.square.reset();
				apu.wave.reset();
				apu.noise.reset();
			}
			break;
		
		default:
			std::cout << "Writing to unknown SoundControl register: NR5" << +reg << std::endl;
			break;
	}
}