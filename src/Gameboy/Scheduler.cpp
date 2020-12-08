#include "Scheduler.h"

#include "Gameboy.h"

void Scheduler::newMCycle() {
	gb.mem.update();
	gb.apu.update();
	gb.ppu.update();
	gb.timer.update();
}