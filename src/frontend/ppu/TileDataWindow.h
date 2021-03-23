#pragma once

#include "core/Gameboy.h"
#include "util/Common.h"
#include "util/ImGuiHeaders.h"
#include "util/Texture.h"

namespace ui {
	class TileDataWindow {
		std::shared_ptr<Gameboy> gb;
		
		bool& show;
		std::array<u32, 128 * 64 * 3> pixels;
		Texture tex;

		u32 zoom = 3;
		bool grid = true;

	public:
		TileDataWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show),
			tex(128, 64 * 3, pixels.data())
		{
			pixels.fill(0xFFFFFFFF);
		}

		void render() {
			if (show) {
				ImGui::Begin("Tile Data", &show);

				gb->ppu.dumpTileMap(pixels);
				tex.update();
				tex.display(
					zoom, 
					grid,
					[](const ImVec2& topleft, const ImVec2& bottomright, u32 mult) {
						const auto boxColor = IM_COL32(105, 105, 105, 255);
						//const auto boxColor = IM_COL32(255, 0, 0, 255);
						const auto boxThickness = 1.5f;

						for (int i = 0; i < 3; ++i) {
							float y = topleft.y + (i * 64 * mult);

							ImVec2 lineL = ImVec2(topleft.x, y),
								lineR = ImVec2(bottomright.x, y);

							ImGui::GetWindowDrawList()->AddLine(lineL, lineR, boxColor, boxThickness);
						}
					}
				);

				ImGui::End();
			}
		}
	};
}