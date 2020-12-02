#pragma once

#include <queue>

#include "Memory.h"
#include "channel/SquareSweep.h"
#include "channel/Square.h"
#include "Util/Common.h"
#include "Util/Math.h"
#include "Util/Types.h"

class APU {
public:
	constexpr static int clock = 1048576;
	constexpr static int frequency = 44100;
	constexpr static int samples = 512;
	constexpr static float amplitude = 0.15f * std::numeric_limits<short>::max();
	constexpr static int maxSampleCycles = 24;

	using SampleBuffer = std::array<u16, samples * 2>;

private:
	constexpr static int maxSequencerCycles = clock / 512;

	u8 sequencer = 0;
	u16 sequencerCycles = maxSequencerCycles;
	u8 sampleCycles = maxSampleCycles;
	u16 bufferOffset = 0;

	SampleBuffer sampleBuffer;

	friend Memory;
	friend SoundControl;
	SoundControl soundControl;
	SquareSweep channel1;
	Square channel2;
public:
	std::queue<SampleBuffer> bufferQueue;

	APU() : soundControl(*this), channel1(soundControl), channel2(soundControl) {
		clean();
	}

	void clean();

	//called in Scheduler::newMCycle
	void update();
};