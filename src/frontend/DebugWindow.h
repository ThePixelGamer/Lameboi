#pragma once

#include "debug/CPUWindow.h"
#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"

class Gameboy;

namespace ui {

class DebugWindow {
	Gameboy& gb;

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

	DebugWindow(Gameboy& gb) :
		gb(gb),
		cpuWindow(gb, show_cpu),
		memWindow(gb, show_memory),
		breakpointsWindow(gb, show_breakpoints)
	{}

	void render();
};

} // namespace ui 