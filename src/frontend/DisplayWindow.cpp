#include "DisplayWindow.h"

namespace ui {

void DisplayWindow::render() {
	if (!show) {
		return;
	}

	// todo: combine this with the bgmapwindow one (both are the same currently)
	auto square = [](ImGuiSizeCallbackData* data) {
		auto beforeImage = static_cast<ImVec2*>(data->UserData);
		ImGuiContext& g = *GImGui;
		ImVec2& padding = ImGui::GetStyle().WindowPadding;
		ImVec2 size = data->DesiredSize - *beforeImage - padding;
		size /= ImVec2(displayWidth, displayHeight);

		float m = std::max(size.x, size.y);

		// todo: open an issue for imgui to provide some sort of context for direction
		if (g.NavInputSource == ImGuiInputSource_None) {
			bool resized = true;
			
			// will break with ImGuiConfigFlags_NoMouseCursorChange
			switch (g.MouseCursor) {
				case ImGuiMouseCursor_ResizeEW:
					m = size.x;
					break;

				case ImGuiMouseCursor_ResizeNS:
					m = size.y;
					break;

				case ImGuiMouseCursor_ResizeNWSE:
				case ImGuiMouseCursor_ResizeNESW:
					break;

				default:
					resized = false;
			}

			if (resized && g.IO.KeyShift)
				m = std::round(m);
		}
		else {
			ImVec2 nav_resize_delta;

			if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
				nav_resize_delta = ImGui::GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiNavReadMode_Down);
			if (g.NavInputSource == ImGuiInputSource_Gamepad)
				nav_resize_delta = ImGui::GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadDPad, ImGuiNavReadMode_Down);

			if (nav_resize_delta.x != 0.0f && nav_resize_delta.y != 0.0f) {
				// m is already set
			}
			else if (nav_resize_delta.x != 0.0f) {
				m = size.x;
			}
			else if (nav_resize_delta.y != 0.0f) {
				m = size.y;
			}
		}

		data->DesiredSize.x = (m * displayWidth) + beforeImage->x + padding.x;
		data->DesiredSize.y = (m * displayHeight) + beforeImage->y + padding.y;
	};

	ImVec2 windowMinSize = oldCursor + ImVec2(displayWidth, displayHeight) + ImGui::GetStyle().WindowPadding;
	ImGui::SetNextWindowSizeConstraints(windowMinSize, ImVec2(FLT_MAX, FLT_MAX), square, &oldCursor);

	ImGui::Begin("Lameboi", &show, ImGuiWindowFlags_NoScrollbar);

	if (ImGui::Button("Stop")) {
		gb.debug.running = false;
		gb.stop();
	}

	ImGui::SameLine(); ImGui::Checkbox("Use Custom Graphics", &useCG);

	oldCursor = ImGui::GetCursorPos();

	// Inputs
	focused = ImGui::IsWindowFocused();

	// Display
	updateBuffer();
	display.update();

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	display.render(std::max(std::min(availSize.x / displayWidth, availSize.y / displayHeight), 1.0f));

	// handle right click on image
	ImGui::OpenPopupOnItemClick("display_context_popup", ImGuiMouseButton_Right);

	if (ImGui::BeginPopupContextItem("display_context_popup")) {
		if (ImGui::Selectable("Fullscreen"));
		if (ImGui::Selectable("Maintain aspect ratio"));
		if (ImGui::Selectable("Maintain square ratio"));
		ImGui::EndPopup();
	}

	ImGui::End();
}

void DisplayWindow::updateBuffer() {
	std::unique_lock lock(ppu.vblank_m);

	if (useCG) {
		ppu.spriteManager.render(displayBuf);
	}
	else {
		ppu.render(displayBuf);
	}
}

} // namespace ui 