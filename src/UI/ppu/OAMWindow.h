#pragma once

#include "Gameboy/Gameboy.h"
#include "Util/Common.h"
#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

namespace ui {
	class OAMWindow {
		std::shared_ptr<Gameboy> gb;
		
		bool& show;
		std::array<u32, 64 * 40> pixels;
		Texture tex;

		u32 zoom = 3;
		bool grid = true;
		u32 step = 1;

	public:
		OAMWindow(std::shared_ptr<Gameboy> gb, bool& show) : 
			gb(gb),
			show(show),
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

				//Need to switch this to a proper 4 slider RGBA
				ImGui::InputScalar("Invisible Color", ImGuiDataType_U32, &gb->ppu.invisPixel, &step, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);

				ImGui::End();
			}
		}
	};
}