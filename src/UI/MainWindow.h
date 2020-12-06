#pragma once

#include <memory>

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "PFD.h"

namespace ui {
	class MainWindow {
		ImGuiIO& io = ImGui::GetIO();

		std::shared_ptr<Gameboy> gb;

	public:
		MainWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb)
		{}

		void render() {
			ImGui::Begin("Lameboi");

			ImGui::Text("UI average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			if (ImGui::Button("Continue")) {
				gb->debug.continuing(true);
			}

			ImGui::SameLine();
			if (ImGui::Button("Pause")) {
				gb->debug.continuing(false);
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				gb->debug.continuing(false);
				gb->running = false;
			}

			ImGui::End();
		}
	};
}