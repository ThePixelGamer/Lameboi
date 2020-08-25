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

			std::unique_lock lock(gb->ppu.vblank_m);

			if (gb->ppu.presenting) {
				tex.mData = gb->ppu.displayPresent.data();
			}
			else {
				tex.mData = gb->ppu.display.data();
			}

			tex.update();
			tex.display(zoom, grid);

			/*
			gb->ppu.isVblank = true;
			gb->ppu.vblank.notify_one();
			*/

			ImGui::End();
		}
	};
}