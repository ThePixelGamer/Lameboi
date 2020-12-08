#pragma once

#include "Util/Types.h"

struct Gameboy;

//relies on the cpu calling for every m-cycle
class Scheduler {
	Gameboy& gb;

public:
	Scheduler(Gameboy& gb) : gb(gb) {}
	void newMCycle();
};