#pragma once

#include <imgui.h>
#include "core/Gameboy.h"

namespace ui {

class CPUWindow {
private:
	Gameboy& gb;

public:
	bool show = false;

	CPUWindow(Gameboy& gb) :
		gb(gb)
	{}

	void render() {
		if (show) {
			ImGui::Begin("CPU", &show);

			static ImU16 step = 1, stepFast = 50;
			ImGui::InputScalar("Program Counter", ImGuiDataType_U16, &gb.cpu.PC, &step, &stepFast, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::InputScalar("Stack Pointer", ImGuiDataType_U16, &gb.cpu.SP, &step, &stepFast, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::Text("0x%02X", gb.cpu.opcode); ImGui::SameLine(); ImGui::Text("Opcode");
			ImGui::Text("Z:%d N:%d HC:%d C:%d", gb.cpu.F.Z, gb.cpu.F.N, gb.cpu.F.HC, gb.cpu.F.C); ImGui::SameLine(); ImGui::Text("Flags");
			ImGui::Text("0x%02X", gb.cpu.A); ImGui::SameLine(); ImGui::Text("A");
			ImGui::Text("0x%04X", gb.cpu.BC); ImGui::SameLine(); ImGui::Text("BC");
			ImGui::Text("0x%04X", gb.cpu.DE); ImGui::SameLine(); ImGui::Text("DE");
			ImGui::Text("0x%04X", gb.cpu.HL); ImGui::SameLine(); ImGui::Text("HL");

			ImGui::End();
		}
	}
};

} // namespace ui 