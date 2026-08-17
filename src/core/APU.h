#pragma once

#include <SDL3/SDL_audio.h>

#include "channel/SquareSweep.h"
#include "channel/Square.h"
#include "channel/Wave.h"
#include "channel/Noise.h"
#include "util/Types.h"

class Memory;

class APU {
private:
	constexpr static int clock = 1048576;
	constexpr static int frequency = 44100;
	constexpr static int samples = 512;
	constexpr static u8 channels = 2;
	constexpr static float volumeModifier = 0.5f; 

	constexpr static int maxSampleCycles = clock / frequency;
	constexpr static int maxSequencerCycles = clock / 512;

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

	bool soundOn;
	u8 sequencerStep;

	// UI
	bool channel1On;
	bool channel2On;
	bool channel3On;
	bool channel4On;

public:
	SquareSweep squareSweep;
	Square square;
	Wave wave;
	Noise noise;

	APU();
	void install(Memory& bus);
	~APU();

	void clean();

	//called in Scheduler::newMCycle
	void update();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);

private:
	float getL() {
		return squareSweep.getL() + square.getL() + wave.getL() + noise.getL();
	}

	float getR() {
		return squareSweep.getR() + square.getR() + wave.getR() + noise.getR();
	}
};
