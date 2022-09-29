#include "DebugMenu.h"

namespace ui {

DebugMenu::DebugMenu(Gameboy& gb, bool& showDebug) :
	debug(gb.debug),
	showDebug(showDebug) {

	ppuMenu = std::make_unique<PPUMenu>(gb);
}

void DebugMenu::render() {
	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Show Debugger")) {
			showDebug = true;
			debug.running = false;
		}

		ppuMenu->render();

		ImGui::EndMenu();
	}

	ppuMenu->renderWindows();
}

} // namespace ui