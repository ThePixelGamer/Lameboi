#pragma once

#include "debug/CPUWindow.h"
#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"

class Debugger;

namespace ui {

class DebugWindow {
	Debugger& debug;

	CPUWindow cpuWindow;
	MemoryWindow memWindow;
	BreakpointsWindow breakpointsWindow;

	bool show_cpu = false;
	bool show_memory = false;
	bool show_breakpoints = false;

	u64 step = 1;
	size_t steps = 1;

public:
	bool show = false;

	DebugWindow(Gameboy& gb);

	void render();
};

} // namespace ui 