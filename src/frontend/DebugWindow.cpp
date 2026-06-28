#include "DebugWindow.h"

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

namespace ui {

DebugWindow::DebugWindow(Gameboy& gb) :
	debug(gb.debug),
	cpuWindow(gb, show_cpu),
	memWindow(gb, show_memory),
	breakpointsWindow(gb, show_breakpoints)
{}

void DebugWindow::render() {
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
			debug.step();
		}

		if (ImGui::Button("Step")) {
			debug.step(steps);
		}

		static ImU64 step = 1, stepFast = 50;
		ImGui::SameLine(); ImGui::InputScalar("##step", ImGuiDataType_U64, &steps, &step, &stepFast);

		if (ImGui::Button("Step Frame")) {
			debug.breakVblank = true;
			debug.resume();
		}

		ImGui::Checkbox("Show PPU Window", &PPU::windowEnabled);
		ImGui::Checkbox("Show PPU Sprites", &PPU::spritesEnabled);

		ImGui::End();
	}
}

} // namespace ui