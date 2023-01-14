#include "BGMapWindow.h"

#include "core/Gameboy.h"
#include "frontend/widgets/SquareResize.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "util/ImGuiHeaders.h"

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

void BGMapWindow::drawExtra(void* extraData, const ImVec2& topleft, const ImVec2& bottomright, float mult) {
	if (!extraData) {
		LB_ERROR(Frontend, "extraData is nullptr");
		return;
	}
	
	auto pThis = static_cast<BGMapWindow*>(extraData);

	if (pThis->displayOutline) {
		drawDisplayBox(pThis->gb, topleft, bottomright, mult);
	}

	if (pThis->selected) {
		Pos2& min = pThis->selectionMin;
		Pos2& max = pThis->selectionMax;

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
		ImVec2 windowMinSize = oldCursor + ImVec2(texWidth, texHeight) + ImGui::GetStyle().WindowPadding;
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
		
		if (selected) {
			const char* popupName = "Configure dump";

			if (ImGui::Button("Dump")) {
				ImGui::OpenPopup(popupName);
			}

			int width = (selectionMax.x - selectionMin.x + 1) * 8;
			int height = (selectionMax.y - selectionMin.y + 1) * 8;
			dumpPreview.setSize(width, height);
			gb.ppu.dumpBGMapTiles(dumpPixels, selectionMin, selectionMax, b_bgmap, b_tileset);
			dumpPreview.update();
			float zoom = std::max(std::min(256.0f / width, 256.0f / height), 1.0f);
			dumpPreview.render(zoom * 0.5f);

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal(popupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				dumpPreview.render(zoom);

				ImGui::Text("Output Directory: %s", "");

				ImGui::Separator();
				if (ImGui::Button("Dump")) { 
					ImGui::CloseCurrentPopup(); 
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
				
				ImGui::EndPopup();
			}
		}

		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();

		oldCursor = ImGui::GetCursorPos();

		gb.ppu.dumpBGMap(pixels, b_bgmap, b_tileset);
		tex.update();

		ImVec2 availSize = ImGui::GetContentRegionAvail();
		float zoom = std::max(std::min(availSize.x / texWidth, availSize.y / texHeight), 1.0f);
		auto [clicked, posx, posy] = tex.render(zoom, true, &drawExtra, this);

		if (clicked) {
			handleClick(posx, posy);
		}
		ImGui::EndGroup();

		ImGui::End();
	}
}

} // namespace ui