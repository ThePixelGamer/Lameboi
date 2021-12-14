#include "MainMenu.h"

namespace ui {

// todo: run on a separate thread to not be affected by UI performance?
void MainMenu::updateFPS() {
	perf = std::chrono::duration_cast<second>(clock::now() - perfTimer);
	if (perf.count() >= 1) {
		perfTimer = clock::now();
		fps = gb.ppu.framesPresented;
		gb.ppu.framesPresented = 0;
	}
}

void MainMenu::render() {
	if (ImGui::BeginMainMenuBar()) {
		fileMenu->render();

		if (ImGui::BeginMenu("Gameboy")) {
			if (ImGui::MenuItem("Pause", nullptr, &paused)) {
				gb.debug.continuing(!paused);
			}

			if (ImGui::MenuItem("Show Display")) {
				context.showDisplay = true;
			}

			if (ImGui::MenuItem("Show Viewport")) {
				context.showViewport = true;
			}

			if (ImGui::MenuItem("Settings")) {
				context.showSettings = true;
			}

			ImGui::EndMenu();
		}

		debugMenu->render();

		ImGui::EndMainMenuBar();
	}

	// todo: move to it's own class or perhaps have the "status bar" be tied to the display window
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), window_flags)) {
		if (ImGui::BeginMenuBar()) {
			updateFPS();

			char v_str[64]{};
			ImFormatString(v_str, IM_ARRAYSIZE(v_str), "%lld FPS", fps);

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x - ImGui::CalcTextSize(v_str).x - ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text(v_str);
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
}

} // namespace ui