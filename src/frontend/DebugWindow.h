#pragma once

#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"
#include "debug/VramWindow.h"

class Debugger;

namespace ui {

class DebugWindow {
	Gameboy& gb;

	bool showCPU = false;
	MemoryWindow mem;
	BreakpointsWindow breakpoints;
	VramWindow vram;

	size_t steps = 1;

public:
	bool show = false;

	DebugWindow(Gameboy& gb);

	void render();

	void renderMenu() {
		if (ImGui::MenuItem("Show Debugger", nullptr, show)) {
			show = true;
			gb.debug.pause();
		}

		if (ImGui::MenuItem("Show Vram Viewer", nullptr, vram.show)) {
			vram.show = true;
		}
	}
};

} // namespace ui 
