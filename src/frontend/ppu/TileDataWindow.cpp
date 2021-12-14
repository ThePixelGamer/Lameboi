#include "TileDataWindow.h"

namespace ui {

void TileDataWindow::render() {
	if (show) {
		ImGui::Begin("Tile Data", &show);

		gb.ppu.dumpTileMap(pixels);
		tex.update();

		auto drawSectionSeparator = [](const ImVec2& topleft, const ImVec2& bottomright, float mult) {
			const auto boxColor = IM_COL32(105, 105, 105, 255);
			//const auto boxColor = IM_COL32(255, 0, 0, 255);
			const auto boxThickness = 1.5f;

			for (int i = 0; i < 3; ++i) {
				float y = topleft.y + (i * 64.0f * mult);

				ImVec2 lineL = ImVec2(topleft.x, y),
					lineR = ImVec2(bottomright.x, y);

				ImGui::GetWindowDrawList()->AddLine(lineL, lineR, boxColor, boxThickness);
			}
		};

		tex.render(zoom, grid, drawSectionSeparator);

		ImGui::End();
	}
}

} // namespace ui