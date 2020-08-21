#pragma once

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

namespace ui {
	class DisplayWindow {
		std::shared_ptr<Gameboy> gb; 
		
		Texture tex;

		//maybe do something with these?
		u32 zoom = 3;
		bool grid = false;

	public:
		DisplayWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb),
			tex(160, 144, gb->ppu.display.data())
		{}

		void render() {
			ImGui::Begin("Display");

			tex.update();
			tex.display(zoom, grid);

			std::unique_lock lock(gb->ppu.vblank_m);
			gb->ppu.isVblank = true;
			gb->ppu.vblank.notify_one();

			ImGui::End();
		}
	};
}