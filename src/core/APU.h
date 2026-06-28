#pragma once

#include <SDL3/SDL_audio.h>

#include "channel/SquareSweep.h"
#include "channel/Square.h"
#include "channel/Wave.h"
#include "channel/Noise.h"
#include "util/Types.h"

class APU {
private:
	constexpr static int clock = 1048576;
	constexpr static int frequency = 44100;
	constexpr static int samples = 512;
	constexpr static u8 channels = 2;
	constexpr static float volumeModifier = 0.5f; 

	constexpr static int maxSampleCycles = clock / frequency;
	constexpr static int maxSequencerCycles = clock / samples;

	u16 sequencerCycles;
	u8 sampleCycles;
	u16 bufferOffset;

	SDL_AudioStream* audio_device;
	std::array<float, samples * 2> sampleBuffer;

	//AudioFile<float> noiseWav;
	//size_t wavePos;

	// FF24 Channel control / ON-OFF / Volume
	u8 rightVolume; // right headphone
	bool vinRight;
	u8 leftVolume; // left headphone
	bool vinLeft;

	SquareSweep squareSweep;
	Square square;
	Wave wave;
	Noise noise;

	bool soundOn;
	u8 sequencerStep;

	// UI
	bool channel1On;
	bool channel2On;
	bool channel3On;
	bool channel4On;

public:
	APU();
	~APU();

	void clean();

	//called in Scheduler::newMCycle
	void update();

	u8 read_reg(u8 reg);
	void write_reg(u8 reg, u8 value);

	u8 read_wave(u8 offset) { return wave.readPattern(offset); }
	void write_wave(u8 offset, u8 value) { return wave.writePattern(offset, value); }

private:
	void resetRegs() {
		rightVolume = 0;
		vinRight = false;
		leftVolume = 0;
		vinLeft = false;

		squareSweep.reset();
		square.reset();
		wave.reset();
		noise.reset();
	}

	void sequence() {
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

	void step() {
		squareSweep.update();
		square.update();
		wave.update();
		noise.update();
	}

	float getL() {
		float output = 0.0f;
		if (channel1On && squareSweep.left) output += squareSweep.sample();
		if (channel2On && square.left)      output += square.sample();
		if (channel3On && wave.left)        output += wave.sample();
		if (channel4On && noise.left)       output += noise.sample();
		return output;
	}

	float getR() {
		float output = 0.0f;
		if (channel1On && squareSweep.right) output += squareSweep.sample();
		if (channel2On && square.right)      output += square.sample();
		if (channel3On && wave.right)        output += wave.sample();
		if (channel4On && noise.right)       output += noise.sample();
		return output;
	}
};