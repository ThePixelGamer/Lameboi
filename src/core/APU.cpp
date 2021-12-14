#include "APU.h"

#include "Config.h"

#include <algorithm>

APU::APU() {
	clean();

	noiseWav.setNumChannels(channels);
	noiseWav.setNumSamplesPerChannel(samples);

	SDL_AudioSpec spec;
	SDL_zero(spec);
	spec.freq = frequency;
	spec.format = AUDIO_F32SYS;
	spec.channels = channels;
	spec.samples = samples;
	spec.callback = nullptr;

	audio_device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
	if (audio_device == 0) {
		std::cout << "Failed to open audio: " << SDL_GetError() << std::endl;
	}
	else {
		SDL_PauseAudioDevice(audio_device, 0);
	}
}

APU::~APU() {
	SDL_CloseAudioDevice(audio_device);
}

void APU::clean() {
	sequencerCycles = maxSequencerCycles;
	sampleCycles = maxSampleCycles;
	bufferOffset = 0;
	sampleBuffer.fill(0.0f);

	control.reset();

	//noiseWav.save("gb-ch4.wav");
	wavePos = 0;
}

// called every 1mhz by the cpu
void APU::update() {
	if (--sequencerCycles == 0) {
		sequencerCycles = maxSequencerCycles;

		control.sequence();
	}

	// 2mhz
	for (int i = 0; i < 2; i++) {
		control.step();
	}

	// mix samples and push it to the buffer
	{
		size_t offset = bufferOffset * channels;
		size_t wavOffset = bufferOffset + (wavePos * samples);

		float volume = 0.0f;
		u8 activeChannelCount = control.channel1On + control.channel2On + control.channel3On + control.channel4On;

		auto adjustVolume = [&](float sample, u8 channelCount) {
			return (sample / channelCount) * ((volume + 1.0f) / 8.0f) * volumeModifier * (config.volume / 100.0f);
		};

		// left
		volume = control.leftVolume;

		/*
		if (control.noise.left)
			noiseWav.samples[0][wavOffset] = adjustVolume(control.noise.sample(), 1);
		*/

		sampleBuffer[offset] += adjustVolume(control.getOutput(false), activeChannelCount);


		// right
		volume = control.rightVolume;

		/*
		if (control.noise.right)
			noiseWav.samples[1][wavOffset] = adjustVolume(control.noise.sample(), 1);
		*/

		sampleBuffer[offset + 1] += adjustVolume(control.getOutput(true), activeChannelCount);
	}

	if (--sampleCycles == 0) {
		sampleCycles = maxSampleCycles;

		size_t offset = bufferOffset * channels;
		sampleBuffer[offset] /= float(maxSampleCycles);
		sampleBuffer[offset + 1] /= float(maxSampleCycles);

		++bufferOffset;
	}

	if (bufferOffset >= samples) {
		bufferOffset = 0;

		++wavePos;
		//noiseWav.setNumSamplesPerChannel((wavePos + 1) * samples);

		uint32_t len = samples * channels * sizeof(float);
		if (config.audioSync) { 
			SDL_QueueAudio(audio_device, sampleBuffer.data(), len);
			sampleBuffer.fill(0.0f);
			while (SDL_GetQueuedAudioSize(audio_device) > len) {}
		}
		else {
			if (SDL_GetQueuedAudioSize(audio_device) <= len) {
				SDL_QueueAudio(audio_device, sampleBuffer.data(), len);
				sampleBuffer.fill(0.0f);
			}
		}
	}
}

u8 APU::read(u8 reg) {
	if (inRange(reg, 0x10, 0x26)) {
		return control.read(reg);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		return control.wave.readPattern(reg);
	}
	else {
		//log
		return 0xFF;
	}
}

void APU::write(u8 reg, u8 value) {
	if (inRange(reg, 0x10, 0x26)) {
		control.write(reg, value);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		control.wave.writePattern(reg, value);
	}
	else {
		//log
	}
}
