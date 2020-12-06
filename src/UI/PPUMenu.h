#pragma once

#include "ppu/BGMapWindow.h"
#include "ppu/OAMWindow.h"
#include "ppu/TileDataWindow.h"

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

namespace ui {
	class PPUMenu {
		std::shared_ptr<Gameboy> gb;

		bool show_bgmap = false;
		bool show_tiledata = false;
		bool show_oam = false;

		std::unique_ptr<BGMapWindow> bgmapWindow;
		std::unique_ptr<TileDataWindow> tileDataWindow;
		std::unique_ptr<OAMWindow> oamWindow;
	public:
		bool show = false;

		PPUMenu(std::shared_ptr<Gameboy> gb) :
			gb(gb) {
			// Put these on the heap because of their pixel array, todo make a HeapArray class?
			bgmapWindow = std::make_unique<BGMapWindow>(gb, show_bgmap);
			tileDataWindow = std::make_unique<TileDataWindow>(gb, show_tiledata);
			oamWindow = std::make_unique<OAMWindow>(gb, show_oam);
		}

		void render() {
			if (ImGui::BeginMenu("PPU")) {
				ImGui::MenuItem("Background Map", nullptr, &show_bgmap);
				ImGui::MenuItem("Tile Data", nullptr, &show_tiledata);
				ImGui::MenuItem("OAM Sprites", nullptr, &show_oam);

				ImGui::EndMenu();
			}

			// doesn't actually render unless show is set
			bgmapWindow->render();
			tileDataWindow->render();
			oamWindow->render();
		}
	};
}