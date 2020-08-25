#pragma once

#include "Util/Common.h"
#include "Util/Math.h"
#include "Util/Types.h"

#include "Memory.h"
#include <queue>

// need to change this in the future
class Square {
	Memory& mem;
	u16 timer = 0;
	u8 vol = 0;
	short output = 0;
	u8 sequence = 0;

	//until I implement a read/write function in the channels
	friend Memory;
	bool runEnvelope = false;
	int volumeCounter = 0;
	u8 lengthCounter = 0;

	constexpr static short dutyTable[4][8] = {
		{ -1, -1, -1, -1, -1, -1, -1, +1 },
		{ +1, -1, -1, -1, -1, -1, -1, +1 },
		{ +1, -1, -1, -1, -1, +1, +1, +1 },
		{ -1, +1, +1, +1, +1, +1, +1, -1 }
	};

public:
	Square(Memory& mem) : mem(mem) {}

	void update();

	void envelope() {
		if (--volumeCounter <= 0) {
			volumeCounter = mem.NR22.envelopeSweep;

			if (runEnvelope && mem.NR22.envelopeSweep > 0) {
				if (mem.NR22.envelopeDirection) {
					++vol;
				}
				else {
					--vol;
				}

				if (vol == 0 || vol == 15) {
					runEnvelope = false;
				}
			}
		}
	}

	void lengthControl() {
		if (lengthCounter > 0 && mem.NR24.counterSelection) {
			if (--lengthCounter == 0) {
				mem.NR52.sound2On = 0;
			}
		}
	}

	short sample() {
		return output;
	}
};

class SquareWave {
	Memory& mem;
	u16 timer = 0;
	int sweepTimer = 0;
	u16 shadowFrequency = 0;
	u8 vol = 0;
	short output = 0;
	u8 sequence = 0;

	//until I implement a read/write function in the channels
	friend Memory;
	bool runEnvelope = false;
	int volumeCounter = 0;
	u8 lengthCounter = 0;

	constexpr static short dutyTable[4][8] = {
		{ -1, -1, -1, -1, -1, -1, -1, +1 },
		{ +1, -1, -1, -1, -1, -1, -1, +1 },
		{ +1, -1, -1, -1, -1, +1, +1, +1 },
		{ -1, +1, +1, +1, +1, +1, +1, -1 }
	};

public:
	SquareWave(Memory& mem) : mem(mem) {}

	void update();
	void sweep();

	void envelope() {
		if (--volumeCounter <= 0) {
			volumeCounter = mem.NR12.envelopeSweep;

			if (runEnvelope && mem.NR12.envelopeSweep > 0) {
				if (mem.NR12.envelopeDirection) {
					++vol;
				}
				else {
					--vol;
				}

				if (vol == 0 || vol == 15) {
					runEnvelope = false;
				}
			}
		}
	}

	void lengthControl() {
		if (lengthCounter > 0 && mem.NR14.counterSelection) {
			if (--lengthCounter == 0) {
				mem.NR52.sound1On = 0;
			}
		}
	}

	short sample() {
		return output;
	}
};

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

	Memory& mem;
	u8 sequencer = 0;
	u16 sequencerCycles = maxSequencerCycles;
	u8 sampleCycles = maxSampleCycles;
	u16 bufferOffset = 0;

	SampleBuffer sampleBuffer;

	friend Memory;
	u8 c1Volume = 0;
	SquareWave channel1;
	Square channel2;
public:
	std::queue<SampleBuffer> bufferQueue;

	APU(Memory& mem) : mem(mem), channel1(mem), channel2(mem) {
		clean();
	}

	void clean();

	//called in Scheduler::newMCycle
	void update();

private:
	/*
	double squareWave(const double frequency, const double elasped_time) {
		double res = 0.0;

		constexpr int i = 10;
		for (int n = 1; n < i; ++n) {
			double u = (2.0 * n) - 1;
			res += std::sin(2.0 * math::pi * u * frequency * elasped_time) / u;
		}
		
		return (4.0 / math::pi) * res;
	}
	*/

	static double squareWave(const double frequency, const double elasped_time) {
		return math::sign(std::sin(2.0 * math::pi * frequency * elasped_time));
	}
};

inline int samples_played = 0;

inline short generateSinWave(const double amplitude, const double frequency, const double elasped_time) {
	return static_cast<short>(amplitude * std::sin(2.0 * math::pi * frequency * elasped_time));
}