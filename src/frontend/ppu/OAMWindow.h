#pragma once

#include "core/Gameboy.h"
#include "util/Common.h"
#include "util/ImGuiHeaders.h"
#include "util/Texture.h"

namespace ui {
	class OAMWindow {
		std::shared_ptr<Gameboy> gb;
		
		bool& show;
		std::array<u32, 64 * 40> pixels;
		float color[3];
		Texture tex;

		u32 zoom = 3;
		bool grid = true;
		u32 step = 1;

	public:
		OAMWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show),
			color{}, // 0 it out
			tex(64, 40, pixels.data())
		{
			pixels.fill(0xFFFFFFFF);
		}

		void render() {
			if (show) {
				ImGui::Begin("OAM", &show);

				gb->ppu.dumpSprites(pixels);
				tex.update();
				tex.display(zoom, grid);

				ImGui::ColorEdit3("Invisible Color", color);
				gb->ppu.invisPixel = 0xFF;
				gb->ppu.invisPixel |= u8(color[2] * 255.0f) << 8;
				gb->ppu.invisPixel |= u8(color[1] * 255.0f) << 16;
				gb->ppu.invisPixel |= u8(color[0] * 255.0f) << 24;

				ImGui::End();
			}
		}
	};
}