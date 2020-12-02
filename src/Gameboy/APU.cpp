#include "APU.h"

#include <algorithm>
#include "Util/SDLHeaders.h"

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

		//mix samples and push it to the buffer
		size_t offset = bufferOffset * 2ll;

		//left
		float leftOutput = 0.0f;
		int leftVolume = (128 * soundControl.NR50.SO2Volume) / 7;
		
		if (soundControl.NR51.sound1ToSO2) {
			float sample = channel1.sample() / 100.0f;
			SDL_MixAudioFormat((Uint8*)&leftOutput, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), leftVolume);
		}
		
		if (soundControl.NR51.sound2ToSO2) {
			float sample = channel2.sample() / 100.0f;
			SDL_MixAudioFormat((Uint8*)&leftOutput, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), leftVolume);
		}

		sampleBuffer[offset] = static_cast<short>(amplitude * leftOutput);


		//right
		float rightOutput = 0.0f;
		int rightVolume = (128 * soundControl.NR50.SO1Volume) / 7;
		
		if (soundControl.NR51.sound1ToSO1) {
			float sample = channel1.sample() / 100.0f;
			SDL_MixAudioFormat((Uint8*)&rightOutput, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), rightVolume);
		}
		
		if (soundControl.NR51.sound2ToSO1) {
			float sample = channel2.sample() / 100.0f;
			SDL_MixAudioFormat((Uint8*)&rightOutput, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), rightVolume);
		}

		sampleBuffer[offset + 1] = static_cast<short>(amplitude * rightOutput);

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