#pragma once

#include <memory>

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"
#include "PFD.h"

namespace ui {
	class MainWindow {
		ImGuiIO& io = ImGui::GetIO();

		std::shared_ptr<Gameboy> gb;
		u64 perf_count = 60;
		u64 fps = 0;

	public:
		MainWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb)
		{}

		// need to change this to be either based on a clock or add in a const vsync framerate (ex: perfCount -= 60 - Framerate)
		void updateFPS() {
			if (--perf_count == 0) {
				perf_count = io.Framerate;
				fps = gb->ppu.framesPresented;
				gb->ppu.framesPresented = 0;
			}
		}

		void render() {
			ImGui::Begin("Lameboi");

			ImGui::Text("UI average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			
			updateFPS();
			ImGui::Text("Renderer (%lld FPS)", fps);

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