#pragma once

#include "debug/CPUWindow.h"
#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"

class Debugger;

namespace ui {

class DebugWindow {
	Debugger& debug;

	CPUWindow cpu;
	MemoryWindow mem;
	BreakpointsWindow breakpoints;

	size_t steps = 1;

public:
	bool show = false;

	DebugWindow(Gameboy& gb);

	void render();
};

} // namespace ui 