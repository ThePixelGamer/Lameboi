#pragma once

#include <memory>

#include "FileMenu.h"
#include "PPUMenu.h"

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

namespace ui {
	class MainMenu {
		std::shared_ptr<Gameboy> gb;

		bool& show_debug_window;
		bool& show_inputs_window;

		std::unique_ptr<FileMenu> fileMenu;
		std::unique_ptr<PPUMenu> ppuMenu;

	public:
		MainMenu(std::shared_ptr<Gameboy> gb, bool& debug, bool& inputs) :
			gb(gb),
			show_debug_window(debug),
			show_inputs_window(inputs) {
			
			fileMenu = std::make_unique<FileMenu>(gb, debug);
			ppuMenu = std::make_unique<PPUMenu>(gb);
		}

		void render() {
			if (ImGui::BeginMainMenuBar()) {
				fileMenu->render();

				if (ImGui::BeginMenu("Gameboy")) {
					if (ImGui::MenuItem("Show Debugger", nullptr, &show_debug_window)) {
						gb->debug.continuing((show_debug_window) ? false : gb->debug.isContinuing());
					}

					ImGui::MenuItem("Show Inputs", nullptr, &show_inputs_window);
					ImGui::EndMenu();
				}

				ppuMenu->render();

				ImGui::EndMainMenuBar();
			}
		}
	};
}