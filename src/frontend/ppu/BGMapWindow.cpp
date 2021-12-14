#include "BGMapWindow.h"

#include "core/Gameboy.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "util/ImGuiHeaders.h"

namespace ui {

// todo: resize the image when the window is resized, also keep the image squared when doing so (avoid extra whitespace?)
void BGMapWindow::render() {
	if (show) {
		auto square = [](ImGuiSizeCallbackData* data) {
			auto beforeImage = static_cast<ImVec2*>(data->UserData);
			ImGuiContext& g = *GImGui;
			ImVec2& padding = ImGui::GetStyle().WindowPadding;
			ImVec2 size = data->DesiredSize - *beforeImage - padding;
			
			float m = std::max(size.x, size.y);
			switch (g.MouseCursor) {
				case ImGuiMouseCursor_ResizeEW:
					m = size.x;
					break;

				case ImGuiMouseCursor_ResizeNS:
					m = size.y;
					break;

				default: break; // shouldn't happen?
			}

			data->DesiredSize.x = m + beforeImage->x + padding.x;
			data->DesiredSize.y = m + beforeImage->y + padding.y;
		};

		ImVec2 windowMinSize = oldCursor + ImVec2(texWidth, texHeight) + ImGui::GetStyle().WindowPadding;
		ImGui::SetNextWindowSizeConstraints(windowMinSize, ImVec2(FLT_MAX, FLT_MAX), square, &oldCursor);

		ImGui::Begin("Background Maps", &show, ImGuiWindowFlags_NoScrollbar);
		
		ImGui::Checkbox("Show display outline", &displayOutline);

		ImGui::RadioButton("Auto##bg", &bgmap, 0); ImGui::SameLine();
		ImGui::RadioButton("9800##bg", &bgmap, 1); ImGui::SameLine();
		ImGui::RadioButton("9C00##bg", &bgmap, 2);

		ImGui::RadioButton("Auto##ts", &tileset, 0); ImGui::SameLine();
		ImGui::RadioButton("8800##ts", &tileset, 1); ImGui::SameLine();
		ImGui::RadioButton("8000##ts", &tileset, 2);

		oldCursor = ImGui::GetCursorPos();

		// Debug Rects
		{
			ImVec2 topLeft = ImGui::GetWindowPos() + ImGui::GetCursorPos();
			ImGui::GetWindowDrawList()->AddRect(topLeft, ImGui::GetWindowPos() + ImGui::GetContentRegionMax(), IM_COL32(0, 0, 255, 255));
			ImGui::GetWindowDrawList()->AddRect(topLeft, topLeft + ImVec2(texWidth, texHeight), IM_COL32(0, 255, 0, 255));
		}

		gb.ppu.dumpBGMap(pixels, (bgmap) ? (bgmap - 1) : gb.ppu.LCDC.bgMap, (tileset) ? (tileset - 1) : gb.ppu.LCDC.tileSet);
		tex.update();

		auto drawDisplayBox = [this](const ImVec2& topleft, const ImVec2& bottomright, float mult) {
			//const auto boxColor = IM_COL32(105, 105, 105, 255);
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
		};

		RenderTexture::DrawCallback drawCallback = nullptr;
		if (displayOutline) {
			drawCallback = drawDisplayBox;
		}

		ImVec2 availSize = ImGui::GetContentRegionAvail();
		tex.render(std::max(std::min(availSize.x / texWidth, availSize.y / texHeight), 1.0f), true, drawCallback);

		ImGui::End();
	}
}

} // namespace ui