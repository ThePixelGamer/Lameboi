#include "APU.h"

#include "Config.h"

#include "util/Log.h"

APU::APU() :
	squareSweep(soundOn, sequencerStep),
	square(soundOn, sequencerStep),
	wave(soundOn, sequencerStep),
	noise(soundOn, sequencerStep)
{
	clean();

	//noiseWav.setNumChannels(channels);
	//noiseWav.setNumSamplesPerChannel(samples);

	const SDL_AudioSpec spec{ SDL_AUDIO_F32, channels, frequency };
	audio_device = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
	if (audio_device == nullptr) {
		LB_ERROR(Audio, "Failed to open audio: %s", SDL_GetError());
	}
	else {
		SDL_ResumeAudioStreamDevice(audio_device);
	}
}

APU::~APU() {
	SDL_DestroyAudioStream(audio_device);
}

void APU::clean() {
	sequencerCycles = maxSequencerCycles;
	sampleCycles = maxSampleCycles;
	bufferOffset = 0;
	sampleBuffer.fill(0.0f);

	resetRegs();

	soundOn = false;
	sequencerStep = 0;

	channel1On = true;
	channel2On = true;
	channel3On = true;
	channel4On = true;

	//noiseWav.save("gb-ch4.wav");
	//wavePos = 0;
}

// called every 1mhz by the cpu
void APU::update() {
	if (--sequencerCycles == 0) {
		sequencerCycles = maxSequencerCycles;

		sequence();
	}

	// 2mhz
	for (int i = 0; i < 2; i++) {
		step();
	}

	// mix samples and push it to the buffer
	{
		size_t offset = bufferOffset * channels;
		//size_t wavOffset = bufferOffset + (wavePos * samples);

		float volume = 0.0f;
		u8 activeChannelCount = channel1On + channel2On + channel3On + channel4On;

		auto adjustVolume = [](float volume, float sample, u8 channelCount) {
			return (sample / channelCount) * ((volume + 1.0f) / 8.0f) * volumeModifier * (config.volume / 100.0f);
		};

		sampleBuffer[offset] += adjustVolume(leftVolume, getL(), activeChannelCount);
		sampleBuffer[offset + 1] += adjustVolume(rightVolume, getR(), activeChannelCount);
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

		//++wavePos;
		//noiseWav.setNumSamplesPerChannel((wavePos + 1) * samples);

		uint32_t len = samples * channels * sizeof(float);
		if (config.audioSync) {
			SDL_PutAudioStreamData(audio_device, sampleBuffer.data(), len);
			sampleBuffer.fill(0.0f);
			while (SDL_GetAudioStreamQueued(audio_device) > len) {}
		}
		else {
			if (SDL_GetAudioStreamQueued(audio_device) <= len) {
				SDL_PutAudioStreamData(audio_device, sampleBuffer.data(), len);
				sampleBuffer.fill(0.0f);
			}
		}
	}
}

u8 APU::read_reg(u8 reg) {
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
			LB_ERROR(Audio, "Reading from unknown SoundControl register: {}", reg);
			return 0xFF;
	}
}

void APU::write_reg(u8 reg, u8 value) {
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
			LB_ERROR(Audio, "Writing to unknown SoundControl register: {}", reg);
			break;
	}
}
