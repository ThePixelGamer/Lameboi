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

		BGMapWindow bgmapWindow;
		TileDataWindow tileDataWindow;
		OAMWindow oamWindow;
	public:
		bool show = false;

		PPUWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb),
			bgmapWindow(gb, show_bgmap),
			tileDataWindow(gb, show_tiledata),
			oamWindow(gb, show_oam)
		{}

		void render() {
			if (show) {
				ImGui::Begin("PPU", &show);

				if (ImGui::Button("Display Background Map"))
					show_bgmap = !show_bgmap;

				if (ImGui::Button("Display Tile Data"))
					show_tiledata = !show_tiledata;

				if (ImGui::Button("Display OAM"))
					show_oam = !show_oam;

				bgmapWindow.render();
				tileDataWindow.render();
				oamWindow.render();

				ImGui::End();
			}
		}
	};
}