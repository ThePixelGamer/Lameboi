#pragma once

#include "ppu/BGMapWindow.h"
#include "ppu/OAMWindow.h"
#include "ppu/TileDataWindow.h"

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

namespace ui {
	class PPUWindow {
		std::shared_ptr<Gameboy> gb;

		bool show_bgmap = false;
		bool show_tiledata = false;
		bool show_oam = false;

		std::unique_ptr<BGMapWindow> bgmapWindow;
		std::unique_ptr<TileDataWindow> tileDataWindow;
		std::unique_ptr<OAMWindow> oamWindow;
	public:
		bool show = false;

		PPUWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb)
		{
			// Put these on the heap because of their pixel array, todo make a HeapArray class?
			bgmapWindow = std::make_unique<BGMapWindow>(gb, show_bgmap);
			tileDataWindow = std::make_unique<TileDataWindow>(gb, show_tiledata);
			oamWindow = std::make_unique<OAMWindow>(gb, show_oam);
		}

		void render() {
			if (show) {
				ImGui::Begin("PPU", &show);

				if (ImGui::Button("Display Background Map"))
					show_bgmap = !show_bgmap;

				if (ImGui::Button("Display Tile Data"))
					show_tiledata = !show_tiledata;

				if (ImGui::Button("Display OAM"))
					show_oam = !show_oam;

				bgmapWindow->render();
				tileDataWindow->render();
				oamWindow->render();

				ImGui::End();
			}
		}
	};
}