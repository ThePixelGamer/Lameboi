#pragma once

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

namespace ui {
	class BreakpointsWindow {
		std::shared_ptr<Gameboy> gb;

		bool& show;

	public:
		BreakpointsWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show) 
		{}

		void render() {
			if (show) {
				ImGui::Begin("Breakpoints", &show);

				u16 new_breakpoint = 0;

				if (ImGui::InputScalar("PC Break", ImGuiDataType_U16, &new_breakpoint, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
					gb->debug.addBreakpoint(new_breakpoint);
					new_breakpoint = 0;
				}

				ImGui::ListBoxHeader("");

				for (auto i = gb->debug.getBreakpoints().begin(); i != gb->debug.getBreakpoints().end();) {
					std::stringstream ss;
					ss << std::hex << *i;
					std::string i_hex = ss.str();

					if (ImGui::Selectable(i_hex.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)
						&& ImGui::IsMouseDoubleClicked(0)) {
						i = gb->debug.removeBreakpoint(i);
					}
					else {
						++i;
					}
				}

				ImGui::ListBoxFooter();

				ImGui::End();
			}
		}
	};
}