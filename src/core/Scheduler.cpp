#include "Scheduler.h"

#include "Gameboy.h"

void Scheduler::newMCycle() {
	++gb.cpu.opcodeCycleCount;

	gb.mem.update();
	gb.apu.update();
	gb.ppu.update();
	gb.timer.update();
}