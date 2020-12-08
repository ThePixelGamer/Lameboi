#include "SoundControl.h"

#include "../APU.h"

void SoundControl::reset() {
	NR50.SO1Volume = 0;
	NR50.vinToS01 = 0;
	NR50.SO2Volume = 0;
	NR50.vinToS02 = 0;

	NR51.sound1ToSO1 = 0;
	NR51.sound2ToSO1 = 0;
	NR51.sound3ToSO1 = 0;
	NR51.sound4ToSO1 = 0;
	NR51.sound1ToSO2 = 0;
	NR51.sound2ToSO2 = 0;
	NR51.sound3ToSO2 = 0;
	NR51.sound4ToSO2 = 0;
}

u8 SoundControl::read(u8 reg) {
	switch (reg) {
		case 0x24: // NR50
			return (NR50.vinToS02 << 7) | (NR50.SO2Volume << 4) | (NR50.vinToS01 << 3) | (NR50.SO1Volume);

		case 0x25:
			return (NR51.sound1ToSO1) | (NR51.sound2ToSO1 << 1) | (NR51.sound3ToSO1 << 2) | (NR51.sound4ToSO1 << 3) |
				(NR51.sound1ToSO2 << 4) | (NR51.sound2ToSO2 << 5) | (NR51.sound3ToSO2 << 6) | (NR51.sound4ToSO2 << 7);

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
			NR50.SO1Volume = (value);
			NR50.vinToS01 = (value >> 3);
			NR50.SO2Volume = (value >> 4);
			NR50.vinToS02 = (value >> 7);
			break;

		case 0x25:
			NR51.sound1ToSO1 = (value);
			NR51.sound2ToSO1 = (value >> 1);
			NR51.sound3ToSO1 = (value >> 2);
			NR51.sound4ToSO1 = (value >> 3);
			NR51.sound1ToSO2 = (value >> 4);
			NR51.sound2ToSO2 = (value >> 5);
			NR51.sound3ToSO2 = (value >> 6);
			NR51.sound4ToSO2 = (value >> 7);
			break;

		case 0x26: // NR52
			soundOn = (value >> 7);
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
			}
			break;
		
		default:
			std::cout << "Writing to unknown SoundControl register: NR5" << +reg << std::endl;
			break;
	}
}