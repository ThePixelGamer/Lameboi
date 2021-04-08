#include "APU.h"

#include <algorithm>

APU::APU() : soundControl(*this), 
	squareSweep(soundControl), square(soundControl), wave(soundControl), noise(soundControl) {
	clean();

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
	audioSync = true;

	sequencer = 0;
	sequencerCycles = maxSequencerCycles;
	sampleCycles = maxSampleCycles;
	bufferOffset = 0;
	sampleBuffer.fill(0);

	soundControl.reset();
	squareSweep.reset();
	square.reset();
	wave.reset();
	noise.reset();
}

//todo: switch to SDL_mixer or something
void APU::update() {
	if (--sequencerCycles == 0) {
		sequencerCycles = maxSequencerCycles;

		if (!(sequencer & 1)) {
			// Sweep 2/6
			if (sequencer & 2) {
				squareSweep.sweep();
			}

			// Length Control 0/2/4/6
			squareSweep.length.tick();
			square.length.tick();
			wave.length.tick();
			noise.length.tick();
		}

		// Volume Envelope
		if (sequencer == 7) {
			squareSweep.envelope.tick();
			square.envelope.tick();
			noise.envelope.tick();
		}

		if (++sequencer == 8) {
			sequencer = 0;
		}
	}

	noise.update();

	for (u8 cycles = 0; cycles != 2; ++cycles) {
		squareSweep.update();
		square.update();
		wave.update();

		if (--sampleCycles == 0) {
			sampleCycles = maxSampleCycles;

			// mix samples and push it to the buffer
			size_t offset = bufferOffset * channels;

			float output, volume;

			auto outputChannel = [&](bool canOutput, float sample) {
				if (canOutput) {
					output += sample;
				}
			};

			auto adjustVolume = [&]() {
				return (output / 4.0f) * (volume / 7.0f) * baseVolumeModifier * emuVolume;
			};

			// left
			output = 0.0f;
			volume = soundControl.leftVolume;

			outputChannel(soundControl.snd1Left, squareSweep.sample());
			outputChannel(soundControl.snd2Left, square.sample());
			outputChannel(soundControl.snd3Left, wave.sample());
			outputChannel(soundControl.snd4Left, noise.sample());
			
			sampleBuffer[offset] = adjustVolume();


			// right
			output = 0.0f;
			volume = soundControl.rightVolume;

			outputChannel(soundControl.snd1Right, squareSweep.sample());
			outputChannel(soundControl.snd2Right, square.sample());
			outputChannel(soundControl.snd3Right, wave.sample());
			outputChannel(soundControl.snd4Right, noise.sample());

			sampleBuffer[offset + 1] = adjustVolume();

			++bufferOffset;
		}

		if (bufferOffset >= samples) {
			bufferOffset = 0;

			uint32_t len = samples * channels * sizeof(float);
			if (true) { // audio sync
				SDL_QueueAudio(audio_device, sampleBuffer.data(), len);
				while (SDL_GetQueuedAudioSize(audio_device) > len) {}
			}
			else {
				if (SDL_GetQueuedAudioSize(audio_device) <= len) {
					SDL_QueueAudio(audio_device, sampleBuffer.data(), len);
				}
			}
		}
	}
}

u8 APU::read(u8 reg) {
	if (inRange(reg, 0x10, 0x14)) {
		return squareSweep.read(reg);
	}
	else if (inRange(reg, 0x16, 0x19)) {
		return square.read(reg);
	}
	else if (inRange(reg, 0x1A, 0x1E)) {
		return wave.read(reg);
	}
	else if (inRange(reg, 0x20, 0x23)) {
		return noise.read(reg);
	}
	else if (inRange(reg, 0x24, 0x26)) {
		return soundControl.read(reg);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		return wave.readPattern(reg);
	}
	else {
		//log
		return 0xFF;
	}
}

void APU::write(u8 reg, u8 value) {
	if (inRange(reg, 0x10, 0x14)) {
		squareSweep.write(reg, value);
	}
	else if (inRange(reg, 0x16, 0x19)) {
		square.write(reg, value);
	}
	else if (inRange(reg, 0x1A, 0x1E)) {
		wave.write(reg, value);
	}
	else if (inRange(reg, 0x20, 0x23)) {
		noise.write(reg, value);
	}
	else if (inRange(reg, 0x24, 0x26)) {
		soundControl.write(reg, value);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		wave.writePattern(reg, value);
	}
	else {
		//log
	}
}
