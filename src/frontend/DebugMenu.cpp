#include "DebugMenu.h"

namespace ui {

void DebugMenu::render() {
	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Show Debugger")) {
			showDebug = true;
			gb.debug.continuing(false);
		}

		ppuMenu->render();

		ImGui::EndMenu();
	}

	ppuMenu->renderWindows();
}

} // namespace ui