#include "DisplayWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "MainWindow.h"

namespace ui {

DisplayWindow::DisplayWindow(UI& context) :
	context(context),
	display() {
}

void DisplayWindow::render() {
	if (!show) {
		return;
	}

	auto& gb = context.gb;

	// todo: combine this with the bgmapwindow one (both are the same currently)
	auto square = [](ImGuiSizeCallbackData* data) {
		auto beforeImage = static_cast<ImVec2*>(data->UserData);
		ImGuiContext& g = *GImGui;
		ImVec2& padding = ImGui::GetStyle().WindowPadding;
		ImVec2 size = data->DesiredSize - *beforeImage - padding;
		size /= ImVec2(Display::W, Display::H);

		float m = std::max(size.x, size.y);

		// todo: open an issue for imgui to provide some sort of context for direction
		if (g.ActiveIdSource == ImGuiInputSource_Mouse) {
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
				nav_resize_delta = ImGui::GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
			if (g.NavInputSource == ImGuiInputSource_Gamepad)
				nav_resize_delta = ImGui::GetKeyMagnitude2d(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown);

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

		data->DesiredSize.x = (m * Display::W) + beforeImage->x + padding.x;
		data->DesiredSize.y = (m * Display::H) + beforeImage->y + padding.y;
	};

	ImVec2 windowMinSize = oldCursor + ImVec2(Display::W, Display::H) + ImGui::GetStyle().WindowPadding;
	//ImGui::SetNextWindowSizeConstraints(windowMinSize, ImVec2(FLT_MAX, FLT_MAX), square, &oldCursor);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::Begin("Lameboi", &show, ImGuiWindowFlags_NoDecoration);
	ImGui::PopStyleVar();

	ImGui::Spacing();
	ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);
	if (ImGui::Button("Stop")) {
		context.gb.stop();
	}

	ImGui::SameLine(); 
	if (ImGui::Checkbox("Use Custom Graphics", &useCG)) {
		gb.ppu.redraw = true;
	}

	// Update FPS counter
	// todo: run on a separate thread to not be affected by UI performance?
	using namespace std::chrono_literals;
	auto perf = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - perfTimer);
	if (perf >= 1s) {
		// add seconds to existing time_point to avoid missing the next second (handle lost remainder from duration_cast)
		perfTimer += perf;
		if (gb.ppu.framesPresented) {
			// take avg in case this takes longer than 1 second to run again
			fps = gb.ppu.framesPresented / perf.count();
			gb.ppu.framesPresented = 0;
		}
		else {
			fps = 0;
		}

		instrCount = gb.cpu.instrCount;
		gb.cpu.instrCount = 0;
	}

	// Not a fan of this
	std::string status{};
	if (gb.emuRun) {
		status = (gb.debug.running) ? fmt::format("{} ips {} fps", instrCount, fps) : "Paused";
	}
	
	if (!status.empty()) {
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(status.c_str()).x - ImGui::GetStyle().ItemInnerSpacing.x));
		ImGui::Text("%s", status.c_str());
	}

	oldCursor = ImGui::GetCursorPos();

	// Inputs
	focused = ImGui::IsWindowFocused();

	// Display
	updateBuffer();

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	display.render(std::max(std::min(availSize.x / Display::W, availSize.y / Display::H), 1.0f));

	// handle right click on image
	ImGui::OpenPopupOnItemClick("display_context_popup", ImGuiMouseButton_Right);

	if (ImGui::BeginPopupContextItem("display_context_popup")) {
		if (ImGui::Selectable("Fullscreen"))
			;
		if (ImGui::Selectable("Maintain aspect ratio"))
			;
		if (ImGui::Selectable("Maintain square ratio"))
			;
		ImGui::EndPopup();
	}

	ImGui::End();
}

void DisplayWindow::updateBuffer() {
	auto& ppu = context.gb.ppu;

	ppu.render([&](Framebuffer& buffer)  {
		if (useCG) {
			renderCG(buffer);
		}
		else {
			for (size_t p = 0; p < (display.W * display.H); ++p) {
				const Color* pixel = &ppu.paletteColors[buffer.pixels[p].color];

				size_t idx = p * 4;
				display.data()[idx] = pixel->r;
				display.data()[idx + 1] = pixel->g;
				display.data()[idx + 2] = pixel->b;
				display.data()[idx + 3] = pixel->a;
			}
		}
	});
}

void DisplayWindow::renderCG(Framebuffer& buffer) {
	auto& ppu = context.gb.ppu;
	
	for (size_t p = 0; p < (160 * 144); ++p) {
		const Color* pixel = [&, &pixel = buffer.pixels[p]]() {
			// todo: either clear the screen when bios -> game or use the bios as a fallback
			auto pTile = ppu.getTile(pixel.tile);
			if (!pTile) {
				return &PPU::paletteColors[pixel.color];
			}
			
			auto& tile = *pTile;
			auto col = &tile.data[pixel.x + (pixel.y * 8)];
			if (tile.usesIndexColors) {
				for (u8 i = 0; i < indexColors.size(); ++i) {
					if (indexColors[i] == *col) {
						return &PPU::paletteColors[pixel.palette[i]];
					}
				}
			}
			return col;
		}();

		size_t idx = p * 4;
		display.data()[idx] = pixel->r;
		display.data()[idx + 1] = pixel->g;
		display.data()[idx + 2] = pixel->b;
		display.data()[idx + 3] = pixel->a;
	}
}

} // namespace ui 
