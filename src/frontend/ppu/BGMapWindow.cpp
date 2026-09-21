#include "BGMapWindow.h"

#include <lodepng.h>

#include "core/Gameboy.h"
#include "frontend/widgets/SquareResize.h"

#include <imgui_stdlib.h>

namespace ui {

void drawDisplayBox(Gameboy& gb, const ImVec2& topleft, const ImVec2& bottomright, float mult) {
	const auto boxColor = IM_COL32(255, 0, 0, 255);
	const auto boxThickness = 1.5f;

	float scx = topleft.x + (gb.ppu.SCX * mult);
	float scy = topleft.y + (gb.ppu.SCY * mult);

	float boxXSize = topleft.x +
		mult * ((gb.ppu.SCX < 96) ? (gb.ppu.SCX + 160) : (gb.ppu.SCX - 96));

	float boxYSize = topleft.y +
		mult * ((gb.ppu.SCY < 112) ? (gb.ppu.SCY + 144) : (gb.ppu.SCY - 112));

	ImVec2 boxTL = ImVec2(scx, scy),
		boxTR = ImVec2(boxXSize, scy),
		boxBL = ImVec2(scx, boxYSize),
		boxBR = ImVec2(boxXSize, boxYSize);

	ImVec2 boxWrapTR = ImVec2(boxXSize, scy),
		boxWrapBL = ImVec2(scx, boxYSize),
		boxWrapXBR = ImVec2(boxXSize, boxYSize),
		boxWrapYBR = ImVec2(boxXSize, boxYSize);

	//wrap the box on the x axis
	if (gb.ppu.SCX >= 96) {
		boxTR = ImVec2(boxXSize, scy);

		ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, scy), boxTR, boxColor, boxThickness); //top
		ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, boxYSize), boxBR, boxColor, boxThickness); //bottom

		boxWrapTR = ImVec2(bottomright.x, scy);
		boxWrapXBR = ImVec2(bottomright.x, boxYSize);
	}

	//wrap the box on the y axis
	if (gb.ppu.SCY >= 112) {
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

void BGMapWindow::drawExtra(const ImVec2& topleft, const ImVec2& bottomright, float mult) {
	if (displayOutline) {
		drawDisplayBox(gb, topleft, bottomright, mult);
	}

	if (selected) {
		Pos2& min = selectionMin;
		Pos2& max = selectionMax;

		float tileMult = 8.0f * mult;
		ImVec2 TL = ImVec2(topleft.x + (min.x * tileMult), topleft.y + (min.y * tileMult));
		ImVec2 BR = ImVec2(topleft.x + ((max.x + 1) * tileMult), topleft.y + ((max.y + 1) * tileMult));

		ImGui::GetWindowDrawList()->AddRect(TL, BR, IM_COL32(0, 255, 0, 255), 0.0f, 0, 1.5f);
	}
};

void BGMapWindow::handleClick(u32 x, u32 y) {
	if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
		selectionMin = initialClick;
		selectionMax = { x, y };

		if (selectionMax < selectionMin) {
			std::swap(selectionMin, selectionMax);
		}
		else if (selectionMax.x < selectionMin.x) {
			std::swap(selectionMin.x, selectionMax.x);
		}
		else if (selectionMax.y < selectionMin.y) {
			std::swap(selectionMin.y, selectionMax.y);
		}
	}
	else {
		initialClick = selectionMin = selectionMax = { x, y };
	}

	selected = true;
}

// todo: resize the image when the window is resized, also keep the image squared when doing so (avoid extra whitespace?)
void BGMapWindow::render() {
	if (show) {
		ImVec2 windowMinSize = oldCursor + ImVec2(bgmapTex.W, bgmapTex.H) + ImGui::GetStyle().WindowPadding;
		ImGui::SetNextWindowSizeConstraints(windowMinSize, ImVec2(FLT_MAX, FLT_MAX), &squareResize, &oldCursor);

		ImGui::Begin("Background Maps", &show, ImGuiWindowFlags_NoScrollbar);
		
		ImGui::BeginGroup();
		ImGui::Checkbox("Show display outline", &displayOutline);

		ImGui::RadioButton("Auto##bg", &bgmap, 0); ImGui::SameLine();
		ImGui::RadioButton("9800##bg", &bgmap, 1); ImGui::SameLine();
		ImGui::RadioButton("9C00##bg", &bgmap, 2);

		ImGui::RadioButton("Auto##ts", &tileset, 0); ImGui::SameLine();
		ImGui::RadioButton("8800##ts", &tileset, 1); ImGui::SameLine();
		ImGui::RadioButton("8000##ts", &tileset, 2);

		bool b_bgmap = (bgmap) ? (bgmap - 1) : gb.ppu.LCDC.bgMap;
		bool b_tileset = (tileset) ? (tileset - 1) : gb.ppu.LCDC.tileSet;
		
		auto dumpBGMap = [&](std::array<u8, 256 * 256 * 4>& outData, Pos2 min = { 0, 0 }, Pos2 max = { 31, 31 }) {
			auto map = gb.ppu.vram.get() + ((b_bgmap) ? 0x1C00 : 0x1800);
			auto attribute = map + PPU::VRAM_SIZE;

			for (int ty = min.y; ty != max.y + 1; ++ty) {
				for (int tx = min.x; tx != max.x + 1; ++tx) {
					int t = tx + (ty * 32);
					size_t rgbIndex = ((ty - min.y) * 256 * 8) + ((tx - min.x) * 8);
					u16 tile = map[t];
					if (!(tile & 0x80) && !b_tileset) {
						tile += 256;
					}

					gb.ppu.dumpTile(&outData[rgbIndex * 4], 256 * 4, tile, bool(attribute[t] & 0x8));
				}
			}
		};

		if (selected) {
			const char* popupName = "Configure dump";

			if (ImGui::Button("Dump")) {
				ImGui::OpenPopup(popupName);
			}

			int width = (selectionMax.x - selectionMin.x + 1) * 8;
			int height = (selectionMax.y - selectionMin.y + 1) * 8;
			dump.setSize(width, height);
			dumpBGMap(dump, selectionMin, selectionMax);
			float zoom = std::max(std::min(256.0f / width, 256.0f / height), 1.0f);
			dump.render(zoom * 0.5f);

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal(popupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				dump.render(zoom);

				auto& manifest = gb.ppu.getManifest();

				ImGui::InputText("##file", &dumpFile);

				ImGui::Separator();
				if (ImGui::Button("Dump")) {
					if (!dumpFile.empty()) {
						std::vector<u8> pixelData;
						pixelData.resize(width * height * 4);
						for (int y = 0; y < height; ++y)
							std::copy_n(dump.data().begin() + (dump.W * y * 4), width * 4, pixelData.begin() + (width * y * 4));

						const std::string folder = manifest.getPath();
						if (auto error = lodepng::encode(folder + dumpFile + ".png", pixelData, width, height)) {
							LB_ERROR(PPU, "encoder error {}: {}", error, lodepng_error_text(error));
						}

						dumpFile = "";
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					dumpFile = "";
					ImGui::CloseCurrentPopup(); 
				}
				
				ImGui::EndPopup();
			}
		}

		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();

		oldCursor = ImGui::GetCursorPos();

		dumpBGMap(bgmapTex);

		ImVec2 availSize = ImGui::GetContentRegionAvail();
		float zoom = std::max(std::min(availSize.x / bgmapTex.W, availSize.y / bgmapTex.H), 1.0f);
		using namespace std::placeholders;
		bgmapTex.render(zoom, true, { 
			.extra = [&](const ImVec2& topleft, const ImVec2& bottomright, float mult) { drawExtra(topleft, bottomright, mult); },
			.click = [&](u32 x, u32 y) { handleClick(x, y); }
		});

		ImGui::EndGroup();

		ImGui::End();
	}
}

} // namespace ui
