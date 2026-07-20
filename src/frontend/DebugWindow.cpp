#include "DebugWindow.h"

#include <imgui.h>

#include "core/Gameboy.h"

namespace ui {

DebugWindow::DebugWindow(Gameboy& gb) :
	gb(gb),
	mem(gb),
	breakpoints(gb)
{}

void DebugWindow::render() {
	if (show) {
		ImGui::Begin("Debugger", &show);

		if (ImGui::Button("Show CPU"))
			showCPU = !showCPU;

		if (ImGui::Button("Show Memory"))
			mem.show = !mem.show;

		if (ImGui::Button("Show Breakpoints"))
			breakpoints.show = !breakpoints.show;

		if (showCPU) {
			ImGui::Begin("CPU", &showCPU);

			/*
			static ImU16 step = 1, stepFast = 50;
			ImGui::InputScalar("Program Counter", ImGuiDataType_U16, &gb.cpu.PC, &step, &stepFast, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::InputScalar("Stack Pointer", ImGuiDataType_U16, &gb.cpu.SP, &step, &stepFast, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::Text("0x%02X", gb.cpu.opcode); ImGui::SameLine(); ImGui::Text("Opcode");
			ImGui::Text("Z:%d N:%d HC:%d C:%d", gb.cpu.F.Z, gb.cpu.F.N, gb.cpu.F.HC, gb.cpu.F.C); ImGui::SameLine(); ImGui::Text("Flags");
			ImGui::Text("0x%02X", gb.cpu.A); ImGui::SameLine(); ImGui::Text("A");
			ImGui::Text("0x%04X", u16(gb.cpu.BC)); ImGui::SameLine(); ImGui::Text("BC");
			ImGui::Text("0x%04X", u16(gb.cpu.DE)); ImGui::SameLine(); ImGui::Text("DE");
			ImGui::Text("0x%04X", u16(gb.cpu.HL)); ImGui::SameLine(); ImGui::Text("HL");
			*/
			ImGui::End();
		}
		mem.render();
		breakpoints.render();

		if (ImGui::Button("Step 1")) {
			gb.debug.step();
		}

		if (ImGui::Button("Step")) {
			gb.debug.step(steps);
		}

		static ImU64 step = 1, stepFast = 50;
		ImGui::SameLine(); ImGui::InputScalar("##step", ImGuiDataType_U64, &steps, &step, &stepFast);

		if (ImGui::Button("Step Frame")) {
			gb.debug.breakVblank = true;
			gb.debug.resume();
		}

		ImGui::Checkbox("Show PPU Window", &PPU::windowEnabled);
		ImGui::Checkbox("Show PPU Sprites", &PPU::spritesEnabled);

		ImGui::End();
	}
}

} // namespace ui
