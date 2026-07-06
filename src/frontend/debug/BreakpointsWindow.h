#pragma once

#include <sstream>

#include <imgui.h>
#include "core/Gameboy.h"

namespace ui {
	class BreakpointsWindow {
		Gameboy& gb;

	public:
		bool show = false;

		BreakpointsWindow(Gameboy& gb) :
			gb(gb) 
		{}

		void render() {
			if (show) {
				ImGui::Begin("Breakpoints", &show);

				u16 new_breakpoint = 0;

				if (ImGui::InputScalar("PC Break", ImGuiDataType_U16, &new_breakpoint, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
					gb.debug.breakpoints.insert(new_breakpoint);
				}

				if (ImGui::BeginListBox("##breakpoints")) {
					for (auto i = gb.debug.breakpoints.begin(); i != gb.debug.breakpoints.end();) {
						std::stringstream ss;
						ss << std::hex << *i;
						std::string i_hex = ss.str();

						if (ImGui::Selectable(i_hex.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)
							&& ImGui::IsMouseDoubleClicked(0)) {
							i = gb.debug.breakpoints.erase(i);
						}
						else {
							++i;
						}
					}

					ImGui::EndListBox();
				}

				ImGui::End();
			}
		}
	};
}