#pragma once

#include "debug/CPUWindow.h"
#include "debug/MemoryWindow.h"
#include "debug/BreakpointsWindow.h"

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"

namespace ui {
	class DebugWindow {
		std::shared_ptr<Gameboy> gb;

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

		DebugWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb),
			cpuWindow(gb, show_cpu),
			memWindow(gb, show_memory),
			breakpointsWindow(gb, show_breakpoints)
		{}

		void render() {
			if (show) {
				ImGui::Begin("Debugger", &show);

				if (ImGui::Button("Show CPU"))
					show_cpu = !show_cpu;

				if (ImGui::Button("Show Memory"))
					show_memory = !show_memory;

				if (ImGui::Button("Show Breakpoints"))
					show_breakpoints = !show_breakpoints;

				cpuWindow.render();
				memWindow.render();
				breakpointsWindow.render();

				if (ImGui::Button("Step 1")) {
					gb->debug.step(1);
				}

				if (ImGui::Button("Step")) {
					gb->debug.step(steps);
				}

				ImGui::SameLine(); ImGui::InputScalar("", ImGuiDataType_U64, &steps, &step);
				
				ImGui::End();
			}
		}
	};
}