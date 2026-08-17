#include "TileDataWindow.h"

#include <imgui_stdlib.h>
#include <lodepng.h>

#include "core/Gameboy.h"

namespace ui {

void drawSectionSeparator(const ImVec2& topleft, const ImVec2& bottomright, float mult) {
	//const auto boxColor = IM_COL32(105, 105, 105, 255);
	const auto boxColor = IM_COL32(255, 0, 0, 255);
	const auto boxThickness = 1.5f;

	for (int i = 1; i < 3; ++i) {
		float y = topleft.y + (i * 64.0f * mult);

		ImVec2 lineL = ImVec2(topleft.x, y),
			lineR = ImVec2(bottomright.x, y);

		ImGui::GetWindowDrawList()->AddLine(lineL, lineR, boxColor, boxThickness);
	}
};

void TileDataWindow::render() {
	if (show) {
		ImGui::Begin("Tile Data", &show);

		gb.ppu.dumpTileMap(tilemap);
		tilemap.render(zoom, grid, { 
			.extra = drawSectionSeparator,
			.hover = [&](u32 x, u32 y) {
				ImGui::Text("Src: %x", gb.ppu.vram_src_locations[(x * 0x10) + (y * 0x100)]);
			} 
		});
		/*
		if (clicked) {
			printf("Clicked: %d, %d\n", posx, posy);

			if (second) {
				if (posy < y1 || (posy == y1 && posx < x1)) {
					x2 = x1;
					y2 = y1;
					x1 = posx;
					y1 = posy;
				}
				else {
					x2 = posx;
					y2 = posy;
				}

				tiles = (x2 + 1 + (y2 * 16)) - (x1 + (y1 * 16));
				height = minHeight = tiles / 32 + (tiles % 32 != 0);
			}
			else {
				x1 = posx;
				y1 = posy;
				x2 = 0;
				y2 = 0;
			}

			valid = second;
			second = !second;
		}

		if (valid) {
			ImGui::InputInt("Height##tiledump", &height);

			if (height < minHeight) {
				height = minHeight;
			}

			if (tiles % height == 0) {
				int width = tiles / height;
				dumpPreview.setSize(width * 8, height * 8);
				gb.ppu.dumpTiles(dumpPixels, x1, y1, x2, y2, width, height);
				dumpPreview.update();
				dumpPreview.render(3.0f);
			}
		}
		*/

		ImGui::InputText("##file", &dumpFile);
		if (ImGui::Button("Dump")) {
			if (!dumpFile.empty()) {
				std::vector<u8> pixelData;
				size_t width = tilemap.W, height = tilemap.H;
				pixelData.resize(width * height * 4);
				for (int y = 0; y < height; ++y)
					std::copy_n(tilemap.data().begin() + (width * y * 4), width * 4, pixelData.begin() + (width * y * 4));

				const std::string folder = gb.ppu.getManifest().getPath();
				if (auto error = lodepng::encode(folder + dumpFile + ".png", pixelData, width, height)) {
					LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
				}

				dumpFile = "";
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::End();
	}
}

} // namespace ui
