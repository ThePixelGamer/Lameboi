#pragma once

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"

namespace ui {
	class InputWindow {
		std::shared_ptr<Gameboy> gb;

	public:
		bool show = false;
		
		InputWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb)
		{}

		void render() {
			if (show) {
				ImGui::Begin("Inputs", &show);

				if (gb->pad.getButtonState(Button::Up)) {
					ImGui::Text("Up");
				}

				if (gb->pad.getButtonState(Button::Down)) {
					ImGui::Text("Down");
				}

				if (gb->pad.getButtonState(Button::Left)) {
					ImGui::Text("Left");
				}

				if (gb->pad.getButtonState(Button::Right)) {
					ImGui::Text("Right");
				}

				if (gb->pad.getButtonState(Button::B)) {
					ImGui::Text("B");
				}

				if (gb->pad.getButtonState(Button::A)) {
					ImGui::Text("A");
				}

				if (gb->pad.getButtonState(Button::Start)) {
					ImGui::Text("Start");
				}

				if (gb->pad.getButtonState(Button::Select)) {
					ImGui::Text("Select");
				}

				ImGui::End();
			}
		}
	};
}