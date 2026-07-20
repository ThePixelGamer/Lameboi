#pragma once

#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"

class Debugger;

namespace ui {

class DebugWindow {
	Gameboy& gb;

	bool showCPU;
	MemoryWindow mem;
	BreakpointsWindow breakpoints;

	size_t steps = 1;

public:
	bool show = false;

	DebugWindow(Gameboy& gb);

	void render();
};

} // namespace ui 
