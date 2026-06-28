#pragma once

#include <set>

#include "util/Types.h"

class Memory;

class Debugger {
	// todo: figure out what I wanted this for
	Memory& mem;

public:
	bool running = true;

	// todo: would using the interrupt requests make more sense?
	bool inVblank = false;

	// PC breakpoints
	std::set<addr> breakpoints;

	// additional breakpoint conditions
	bool breakVblank = false;
	size_t steps = 0;

	Debugger(Memory& mem) : mem(mem) {}

	void resume() {
		running = true;
	}

	void pause() {
		running = false;
	}

	void step(u64 step = 1) {
		steps = step;
	}

	bool shouldBreak(addr PC);
};