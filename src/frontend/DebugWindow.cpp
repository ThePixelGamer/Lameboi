#include "DebugWindow.h"

#include <imgui.h>

#include "core/Gameboy.h"

namespace ui {

DebugWindow::DebugWindow(Gameboy& gb) :
	debug(gb.debug),
	cpu(gb),
	mem(gb),
	breakpoints(gb)
{}

void DebugWindow::render() {
	if (show) {
		ImGui::Begin("Debugger", &show);

		if (ImGui::Button("Show CPU"))
			cpu.show = !cpu.show;

		if (ImGui::Button("Show Memory"))
			mem.show = !mem.show;

		if (ImGui::Button("Show Breakpoints"))
			breakpoints.show = !breakpoints.show;

		cpu.render();
		mem.render();
		breakpoints.render();

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