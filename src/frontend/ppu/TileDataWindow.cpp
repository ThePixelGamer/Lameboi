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

		
		for (u16 t = 0; t < 0x180; ++t) {
			size_t rgbIndex = ((t / 16) * 128 * 8) + ((t % 16) * 8);

			gb.ppu.dumpTile(&tilemap[rgbIndex * 4], 128 * 4, t, false);
		}

		for (u16 t = 0; t < 0x180; ++t) {
			size_t rgbIndex = ((t / 16) * 128 * 8) + ((t % 16) * 8);

			gb.ppu.dumpTile(&altTilemap[rgbIndex * 4], 128 * 4, t, true);
		}

		tilemap.render(zoom, grid, { 
			.extra = drawSectionSeparator,
			.hover = [&](u32 x, u32 y) {
				ImGui::Text("Src: 0x%x", gb.ppu.vram_src_locations[(x * 0x10) + (y * 0x100)]);
				auto& tile = gb.ppu.getFrontBuffer().meta.tiles[y * 16 + x];
				ImGui::Text("CG Hash: 0x%016llx", tile.hash);
				ImGui::Text("CG Src: 0x%x", tile.src);
			} 
		});

		ImGui::SameLine();
		altTilemap.render(zoom, grid, { 
			.extra = drawSectionSeparator,
			.hover = [&](u32 x, u32 y) {
				ImGui::Text("Src: 0x%x", gb.ppu.vram_src_locations[PPU::VRAM_SIZE + (x * 0x10) + (y * 0x100)]);
				auto& tile = gb.ppu.getFrontBuffer().meta.tiles[384 + y * 16 + x];
				ImGui::Text("CG Hash: 0x%016llx", tile.hash);
				ImGui::Text("CG Src: 0x%x", tile.src);
			} 
		});

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
