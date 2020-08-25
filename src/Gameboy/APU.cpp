#include "APU.h"

#include <algorithm>

void Square::update() {
	if (--timer == 0) {
		timer = 2048 - ((mem.NR24.frequencyHI << 8) | mem.NR23);
		if (++sequence == 8) {
			sequence = 0;
		}
	}

	if (mem.NR52.sound2On) {
		if (dutyTable[mem.NR21.waveDuty][sequence]) {
			outputVolume = vol;
		}
		else {
			outputVolume = 0;
		}
	}
	else {
		outputVolume = 0;
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
			channel2.lengthControl();
		}

		//Sweep
		if (sequencer == 2 || sequencer == 6) {

		}

		//Volume Envelope
		if (sequencer == 7) {
			// Channel 2
			channel2.envelope();
		}

		if (++sequencer == 8) {
			sequencer = 0;
		}
	}

	channel2.update();

	if (--sampleCycles == 0) {
		sampleCycles = maxSampleCycles;

		//mix samples and push it to the temp buffer
		size_t offset = bufferOffset * 2ll;

		//left
		short sample = 0;

		if (mem.NR51.sound2ToSO1) {
			sample += channel2.volume();
		}

		sampleBuffer[offset] = sample;


		//right
		sample = 0;

		if (mem.NR51.sound2ToSO2) {
			sample += channel2.volume();
		}

		sampleBuffer[offset + 1] = sample;

		++bufferOffset;
	}

	if (bufferOffset >= samples) {
		bufferOffset = 0;

		//push temp buffer to queue
		bufferQueue.push(std::move(sampleBuffer));
	}
}

void lengthClock() {
	
}