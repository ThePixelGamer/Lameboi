#pragma once

#include "core/Gameboy.h"
#include "util/Common.h"
#include "util/ImGuiHeaders.h"
#include "util/Texture.h"

namespace ui {
	class BGMapWindow {
		std::shared_ptr<Gameboy> gb;
		
		bool& show;
		std::array<u32, 256 * 256> pixels;
		Texture tex;

		u32 zoom = 2;
		bool grid = true;
		bool bgmap = false;
		bool tileset = true;

		u32 step = 1;
	public:
		BGMapWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show),
			tex(256, 256, pixels.data())
		{
			pixels.fill(0xFFFFFFFF);
		}

		void render() {
			if (show) {
				ImGui::Begin("Background Maps", &show);

				if (ImGui::Button("Background Map"))
					bgmap = !bgmap;
				ImGui::SameLine(); ImGui::Text("0x%04X", (bgmap) ? 0x9C00 : 0x9800);

				if (ImGui::Button("Tile Data"))
					tileset = !tileset;
				ImGui::SameLine(); ImGui::Text("0x%04X", (tileset) ? 0x8000 : 0x8800);

				gb->ppu.dumpBGMap(pixels, bgmap, tileset);
				tex.update();

				tex.display(
					zoom,
					grid,
					[this](const ImVec2& topleft, const ImVec2& bottomright, u32 mult) {
						//const auto boxColor = IM_COL32(105, 105, 105, 255);
						const auto boxColor = IM_COL32(255, 0, 0, 255);
						const auto boxThickness = 1.5f;

						float scx = topleft.x + (gb->ppu.SCX * mult);
						float scy = topleft.y + (gb->ppu.SCY * mult);

						float boxXSize = topleft.x +
							mult * ((gb->ppu.SCX < 96) ? (gb->ppu.SCX + 160) : (gb->ppu.SCX - 96));

						float boxYSize = topleft.y +
							mult * ((gb->ppu.SCY < 112) ? (gb->ppu.SCY + 144) : (gb->ppu.SCY - 112));

						ImVec2 boxTL = ImVec2(scx, scy),
							boxTR = ImVec2(boxXSize, scy),
							boxBL = ImVec2(scx, boxYSize),
							boxBR = ImVec2(boxXSize, boxYSize);

						ImVec2 boxWrapTR = ImVec2(boxXSize, scy),
							boxWrapBL = ImVec2(scx, boxYSize),
							boxWrapXBR = ImVec2(boxXSize, boxYSize),
							boxWrapYBR = ImVec2(boxXSize, boxYSize);

						if (gb->ppu.SCX >= 96) {
							//wrap the box on the x axis
							boxTR = ImVec2(boxXSize, scy);

							ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, scy), boxTR, boxColor, boxThickness); //top
							ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, boxYSize), boxBR, boxColor, boxThickness); //bottom

							boxWrapTR = ImVec2(bottomright.x, scy);
							boxWrapXBR = ImVec2(bottomright.x, boxYSize);
						}

						if (gb->ppu.SCY >= 112) {
							//wrap the box on the y axis
							boxBL = ImVec2(scx, boxYSize);

							ImGui::GetWindowDrawList()->AddLine(ImVec2(scx, topleft.y), boxBL, boxColor, boxThickness); //left
							ImGui::GetWindowDrawList()->AddLine(ImVec2(boxXSize, topleft.y), boxBR, boxColor, boxThickness); //right

							boxWrapBL = ImVec2(scx, bottomright.y);
							boxWrapYBR = ImVec2(boxXSize, bottomright.y);
						}

						ImGui::GetWindowDrawList()->AddLine(boxTL, boxWrapTR, boxColor, boxThickness); //top
						ImGui::GetWindowDrawList()->AddLine(boxTL, boxWrapBL, boxColor, boxThickness); //left
						ImGui::GetWindowDrawList()->AddLine(boxTR, boxWrapYBR, boxColor, boxThickness); //right
						ImGui::GetWindowDrawList()->AddLine(boxBL, boxWrapXBR, boxColor, boxThickness); //bottom
					}
				);

				ImGui::End();
			}
		}
	};
}