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
	short outputVolume = 0;
	u8 sequence = 0;

	//until I implement a read/write function in the channels
	friend Memory;
	bool runEnvelope = false;
	int volumeCounter = 0;
	u8 lengthCounter = 0;

	constexpr static bool dutyTable[4][8] = {
		{false, false, false, false, false, false, false, true},
		{true, false, false, false, false, false, false, true},
		{true, false, false, false, false, true, true, true},
		{false, true, true, true, true, true, true, false}
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

	short volume() {
		return outputVolume;
	}
};

class APU {
public:
	constexpr static int clock = 1048576;
	constexpr static int frequency = 44100;
	constexpr static int samples = 512;
	constexpr static double amplitude = 0.15 * std::numeric_limits<short>::max();
	constexpr static int maxSampleCycles = clock / frequency;

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
	
	Square channel2;
	u8 volumeCounter = 0;
public:
	std::queue<SampleBuffer> bufferQueue;

	APU(Memory& mem) : mem(mem), channel2(mem) {
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

inline void sound_mix(void* userdata, u8* strm, int len) {
	auto apu = reinterpret_cast<APU*>(userdata);
	auto stream = reinterpret_cast<s16*>(strm);

	const auto new_len = (len / sizeof(short)) / 2;
	if (!apu->bufferQueue.empty()) {
		for (int i = 0; i < new_len; i++) {
			size_t pos = i * 2ll;
			stream[pos] = static_cast<short>(APU::amplitude * apu->bufferQueue.front()[pos]);
			stream[pos + 1] = static_cast<short>(APU::amplitude * apu->bufferQueue.front()[pos + 1]);
		}

		apu->bufferQueue.pop();
	}
	else {
		for (int i = 0; i < new_len; i++) {
			stream[i * 2 + 0] = 0;
			stream[i * 2 + 1] = 0;
		}
	}
}