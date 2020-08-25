#include "APU.h"

#include <algorithm>
#include "Util/SDLHeaders.h"

void Square::update() {
	if (--timer == 0) {
		timer = 2048 - ((mem.NR24.frequencyHI << 8) | mem.NR23);
		if (++sequence == 8)
			sequence = 0;
	}

	if (mem.NR52.sound2On) {
		output = dutyTable[mem.NR21.waveDuty][sequence] * vol;
	}
	else {
		output = 0;
	}
}

void SquareWave::update() {
	if (--timer == 0) {
		timer = 2048 - shadowFrequency;
		if (++sequence == 8)
			sequence = 0;
	}

	if (mem.NR52.sound1On) {
		output = dutyTable[mem.NR11.waveDuty][sequence] * vol;
	}
	else {
		output = 0;
	}
}

void SquareWave::sweep() {
	if (--sweepTimer <= 0) {
		sweepTimer = mem.NR10.sweepTime;

		if (mem.NR10.sweepTime != 0 || mem.NR10.sweepShifts != 0) {
			short adjustedFrequency = shadowFrequency >> mem.NR10.sweepShifts;

			if (mem.NR10.sweepDecrease) {
				adjustedFrequency = -adjustedFrequency;
			}

			adjustedFrequency = shadowFrequency + adjustedFrequency;

			if (adjustedFrequency > 2047 || adjustedFrequency < 0) {
				mem.NR52.sound1On = false;
			}
			else {
				shadowFrequency = adjustedFrequency;
				mem.NR13 = shadowFrequency & 0xFF;
				mem.NR14.frequencyHI = shadowFrequency >> 8;
			}
		}
	}
}

void APU::clean() {
	sampleBuffer.fill(0);
}

void APU::update() {
	if (--sequencerCycles == 0) {
		sequencerCycles = maxSequencerCycles;

		//Length Control
		if (sequencer % 2 == 0) {
			channel1.lengthControl();
			channel2.lengthControl();
		}

		//Sweep
		if (sequencer == 2 || sequencer == 6) {
			channel1.sweep();
		}

		//Volume Envelope
		if (sequencer == 7) {
			channel1.envelope();
			channel2.envelope();
		}

		if (++sequencer == 8) {
			sequencer = 0;
		}
	}

	channel1.update();
	channel2.update();

	if (--sampleCycles == 0) {
		sampleCycles = maxSampleCycles;

		//mix samples and push it to the temp buffer
		size_t offset = bufferOffset * 2ll;

		//left
		float output = 0.0f;
		int volume = (128 * mem.NR50.SO2Volume) / 7;

		if (mem.NR51.sound1ToSO2) {
			float sample = static_cast<float>(channel1.sample()) / 100.0f;
			SDL_MixAudioFormat((Uint8*)&output, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), volume);
		}
		if (mem.NR51.sound2ToSO2) {
			float sample = static_cast<float>(channel2.sample()) / 100.0f;
			SDL_MixAudioFormat((Uint8*)&output, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), volume);
		}

		sampleBuffer[offset] = static_cast<short>(amplitude * output);


		//right
		output = 0.0f;
		volume = (128 * mem.NR50.SO1Volume) / 7;

		if (mem.NR51.sound1ToSO1) {
			float sample = static_cast<float>(channel1.sample()) / 100.0f;
			SDL_MixAudioFormat((Uint8*)&output, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), volume);
		}
		if (mem.NR51.sound2ToSO1) {
			float sample = static_cast<float>(channel2.sample()) / 100.0f;
			SDL_MixAudioFormat((Uint8*)&output, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), volume);
		}

		sampleBuffer[offset + 1] = static_cast<short>(amplitude * output);

		++bufferOffset;
	}

	if (bufferOffset >= samples) {
		bufferOffset = 0;

		uint32_t len = samples * sizeof(short) * 2;
		while (SDL_GetQueuedAudioSize(1) > len) {
			SDL_Delay(1);
		}

		SDL_QueueAudio(1, sampleBuffer.data(), len);
	}
}