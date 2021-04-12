#include "SoundControl.h"

#include "../APU.h"

void SoundControl::reset() {
	resetRegs();

	soundOn = false;
	sequencerStep = 0;

	channel1On = true;
	channel2On = true;
	channel3On = true;
	channel4On = true;
}

void SoundControl::resetRegs() {
	rightVolume = 0;
	vinRight = false;
	leftVolume = 0;
	vinLeft = false;

	squareSweep.reset();
	square.reset();
	wave.reset();
	noise.reset();
}

// TODO: add the option to dump the samples to a wav file
float SoundControl::getOutput(bool right) {
	float output = 0.0f;
	
	if (right) {
		if (channel1On && squareSweep.right) output += squareSweep.sample();
		if (channel2On && square.right)      output += square.sample();
		if (channel3On && wave.right)        output += wave.sample();
		if (channel4On && noise.right)       output += noise.sample();
	}
	else {
		if (channel1On && squareSweep.left) output += squareSweep.sample();
		if (channel2On && square.left)      output += square.sample();
		if (channel3On && wave.left)        output += wave.sample();
		if (channel4On && noise.left)       output += noise.sample();
	}

	return output;
}

void SoundControl::sequence() {
	if ((sequencerStep & 1) == 0) {
		// Sweep 2/6
		if (sequencerStep & 2) {
			squareSweep.sweep();
		}

		// Length Control 0/2/4/6
		squareSweep.length.tick();
		square.length.tick();
		wave.length.tick();
		noise.length.tick();
	}

	// Volume Envelope
	if (sequencerStep == 7) {
		squareSweep.envelope.tick();
		square.envelope.tick();
		noise.envelope.tick();
	}

	if (++sequencerStep == 8) {
		sequencerStep = 0;
	}
}

void SoundControl::step() {
	squareSweep.update();
	square.update();
	wave.update();
	noise.update();
}

u8 SoundControl::read(u8 reg) {
	//if (reg)

	switch (reg) {
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
			return squareSweep.read(reg);

		case 0x16: case 0x17: case 0x18: case 0x19:
			return square.read(reg);

		case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
			return wave.read(reg);

		case 0x20: case 0x21: case 0x22: case 0x23:
			return noise.read(reg);

		case 0x24: // NR50
			return (vinLeft << 7) | (leftVolume << 4) | (vinRight << 3) | (rightVolume);

		case 0x25: // NR51
			return u8(squareSweep.right) | (square.right << 1) | (wave.right << 2) | (noise.right << 3) |
				(squareSweep.left << 4) | (square.left << 5) | (wave.left << 6) | (noise.left << 7);

		case 0x26: // NR52
			return (soundOn << 7) | 0x70 | (noise.soundOn << 3) | (wave.soundOn << 2) | (square.soundOn << 1) | u8(squareSweep.soundOn);

		default:
			std::cout << "Reading from unknown SoundControl register: NR5" << +reg << std::endl;
			return 0xFF;
	}
}

void SoundControl::write(u8 reg, u8 value) {
	switch (reg) {
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
			squareSweep.write(reg, value);
			break;

		case 0x16: case 0x17: case 0x18: case 0x19:
			square.write(reg, value);
			break;

		case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
			wave.write(reg, value);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23:
			noise.write(reg, value);
			break;

		case 0x24: // NR50
			if (soundOn) {
				rightVolume = (value & 0x7);
				vinRight = (value & 0x8);
				leftVolume = ((value >> 4) & 0x7);
				vinLeft = (value & 0x80);
			}
			break;

		case 0x25: // NR51
			if (soundOn) {
				squareSweep.right = (value & 0x1);
				square.right = (value & 0x2);
				wave.right = (value & 0x4);
				noise.right = (value & 0x8);
				squareSweep.left = (value & 0x10);
				square.left = (value & 0x20);
				wave.left = (value & 0x40);
				noise.left = (value & 0x80);
			}
			break;

		case 0x26: // NR52
			soundOn = (value & 0x80);
			if (soundOn) {
				sequencerStep = 0;
				squareSweep.resetWaveDuty();
				square.resetWaveDuty();
				wave.resetWaveBuffer();
			}
			else {
				resetRegs();
			}
			break;
		
		default:
			std::cout << "Writing to unknown SoundControl register: NR5" << +reg << std::endl;
			break;
	}
}