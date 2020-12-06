#include "APU.h"

#include <algorithm>

APU::APU() : soundControl(*this), squareSweep(soundControl), square(soundControl), wave(soundControl) {
	clean();

	SDL_AudioSpec spec;
	SDL_zero(spec);
	spec.freq = frequency;
	spec.format = AUDIO_F32SYS;
	spec.channels = 2;
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
	sampleBuffer.fill(0);

	soundControl.reset();
	squareSweep.reset();
	square.reset();
	wave.reset();
}

void APU::update() {
	if (--sequencerCycles == 0) {
		sequencerCycles = maxSequencerCycles;

		//Length Control
		if ((sequencer & 1) == 0) {
			squareSweep.lengthControl();
			square.lengthControl();
			wave.lengthControl();
		}

		//Sweep
		if (sequencer == 2 || sequencer == 6) {
			squareSweep.sweep();
		}

		//Volume Envelope
		if (sequencer == 7) {
			squareSweep.envelope();
			square.envelope();
		}

		if (++sequencer == 8) {
			sequencer = 0;
		}
	}

	for (u8 cycles = 4; cycles != 0; --cycles) {
		squareSweep.update();
		square.update();
		wave.update();

		if (--sampleCycles == 0) {
			sampleCycles = maxSampleCycles;

			//mix samples and push it to the buffer
			size_t offset = bufferOffset * 2ll;

			float output = 0.0f;
			int volume = (128 * soundControl.NR50.SO2Volume) / 7;

			auto outputChannel = [&](bool canOutput, float sample) {
				if (canOutput) {
					sample /= 100.0f;
					SDL_MixAudioFormat((Uint8*)&output, (Uint8*)&sample, AUDIO_F32SYS, sizeof(float), volume);
				}
			};

			//left
			outputChannel(soundControl.NR51.sound1ToSO2, squareSweep.sample());
			outputChannel(soundControl.NR51.sound2ToSO2, square.sample());
			outputChannel(soundControl.NR51.sound3ToSO2, wave.sample());
			sampleBuffer[offset] = amplitude * output;


			//right
			output = 0.0f;
			volume = (128 * soundControl.NR50.SO1Volume) / 7;

			outputChannel(soundControl.NR51.sound1ToSO1, squareSweep.sample());
			outputChannel(soundControl.NR51.sound2ToSO1, square.sample());
			outputChannel(soundControl.NR51.sound3ToSO1, wave.sample());
			sampleBuffer[offset + 1] = amplitude * output;

			++bufferOffset;
		}

		if (bufferOffset >= samples) {
			bufferOffset = 0;

			uint32_t len = samples * 2 * sizeof(float);
			while (SDL_GetQueuedAudioSize(audio_device) > len) {
				SDL_Delay(1);
			}

			SDL_QueueAudio(audio_device, sampleBuffer.data(), len);
		}
	}
}

template <typename T>
bool inRange(const T& value, int low, int high) {
	return (value >= low) && (value <= high);
}

u8 APU::read(u8 reg) {
	if (inRange(reg, 0x10, 0x14)) {
		return squareSweep.read(reg - 0x10);
	}
	else if (inRange(reg, 0x15, 0x19)) {
		return square.read(reg - 0x15);
	}
	else if (inRange(reg, 0x1A, 0x1E)) {
		return wave.read(reg - 0x1A);
	}
	else if (inRange(reg, 0x24, 0x26)) {
		return soundControl.read(reg - 0x24);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		return wave.read(reg);
	}
	else {
		return 0xFF;
	}
}

void APU::write(u8 reg, u8 value) {
	if (inRange(reg, 0x10, 0x14)) {
		squareSweep.write(reg - 0x10, value);
	}
	else if (inRange(reg, 0x15, 0x19)) {
		square.write(reg - 0x15, value);
	}
	else if (inRange(reg, 0x1A, 0x1E)) {
		wave.write(reg - 0x1A, value);
	}
	else if (inRange(reg, 0x24, 0x26)) {
		soundControl.write(reg - 0x24, value);
	}
	else if (inRange(reg, 0x30, 0x3F)) {
		wave.write(reg, value);
	}
}
