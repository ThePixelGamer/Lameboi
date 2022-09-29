#pragma once

#include <set>

#include "util/Types.h"

class Memory;

class Debugger {
	// todo: figure out what I wanted this for
	Memory& mem;

	bool isStepping = false;
	size_t steps = 0;

	std::set<u16> breakpoints;
	using BreakpointIter = std::set<u16>::iterator;

public:
	bool running = true;
	bool inVblank = false;
	bool vblankStep = false;

	Debugger(Memory& mem) : mem(mem) {}

	void addBreakpoint(u16 PC);
	BreakpointIter removeBreakpoint(BreakpointIter PC);
	void removeBreakpoint(u16 PC);
	std::set<u16>& getBreakpoints();

	void step(size_t steps_);
	size_t amountToStep(u16 PC);
};