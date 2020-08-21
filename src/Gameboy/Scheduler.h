#pragma once

#include "Util/Types.h"

struct Gameboy;

//relies on the cpu calling for every m-cycle
class Scheduler {
	Gameboy& gb;
	const u8 cyclesDivNeeds = 64; // takes the amount of m-cycles to increment the div reg
	u8 divCycles = 0;
	u8 currentCycleCount = 0;

public:
	bool handleInterrupts = true;

	Scheduler(Gameboy& gb) : gb(gb) {}

	void clean();
	void resetDiv();
	void newMCycle();
};