#include "DebugWindow.h"

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

namespace ui {

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
			gb.debug.step(1);
		}

		if (ImGui::Button("Step")) {
			gb.debug.step(steps);
		}
		ImGui::SameLine(); ImGui::InputScalar("", ImGuiDataType_U64, &steps, &step);

		ImGui::Checkbox("Show PPU Window", &PPU::windowEnabled);
		ImGui::Checkbox("Show PPU Sprites", &PPU::spritesEnabled);

		ImGui::End();
	}
}

} // namespace ui