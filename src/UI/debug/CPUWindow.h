#pragma once

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"

namespace ui {
	class CPUWindow {
		std::shared_ptr<Gameboy> gb;

		bool& show;
		u16 step = 1;

	public:
		CPUWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show)
		{}

		void render() {
			if (show) {
				ImGui::Begin("CPU", &show);

				ImGui::InputScalar("Program Counter", ImGuiDataType_U16, &gb->cpu.PC, &step, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
				ImGui::InputScalar("Stack Pointer", ImGuiDataType_U16, &gb->cpu.SP, &step, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
				ImGui::Text("0x%02X", gb->cpu.opcode); ImGui::SameLine(); ImGui::Text("Opcode");
				ImGui::Text("Z:%d N:%d HC:%d C:%d", gb->cpu.F.Z, gb->cpu.F.N, gb->cpu.F.HC, gb->cpu.F.C); ImGui::SameLine(); ImGui::Text("Flags");
				ImGui::Text("0x%02X", gb->cpu.A); ImGui::SameLine(); ImGui::Text("A");
				ImGui::Text("0x%04X", gb->cpu.BC); ImGui::SameLine(); ImGui::Text("BC");
				ImGui::Text("0x%04X", gb->cpu.DE); ImGui::SameLine(); ImGui::Text("DE");
				ImGui::Text("0x%04X", gb->cpu.HL); ImGui::SameLine(); ImGui::Text("HL");

				ImGui::End();
			}
		}
	};
}