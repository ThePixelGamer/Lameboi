#pragma once

#include <queue>
#include <SDL_audio.h>

#include "Memory.h"
#include "channel/SoundControl.h"
#include "util/Common.h"
#include "util/AudioFile.h"
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

	SDL_AudioDeviceID audio_device;
	std::array<float, samples * 2> sampleBuffer;

	AudioFile<float> noiseWav;
	size_t wavePos;

	SoundControl control;

public:
	APU();
	~APU();

	void clean();

	//called in Scheduler::newMCycle
	void update();

	u8 read(u8 reg);
	void write(u8 reg, u8 value);
};