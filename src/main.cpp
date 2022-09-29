#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "core/Gameboy.h"
#include "core/Input.h"

#include "frontend/DebugWindow.h"
#include "frontend/DisplayWindow.h"
#include "frontend/SettingsWindow.h"
#include "frontend/ViewportWindow.h"
#include "frontend/MainMenu.h"

#include "util/FileUtil.h"
#include "util/SDLInterface.h"

int main(int, char**) {
	SDLInterface loader;
	if (!loader.init()) {
		return 1;
	}

	Gameboy* gb = new Gameboy;

	ui::DisplayWindow display(*gb);
	ui::DebugWindow debug(*gb);
	ui::SettingsWindow settings(*gb);
	ui::ViewportWindow viewport;
	ui::MainMenu menubar(*gb, { display.show, viewport.show, debug.show, settings.show, loader });

	// create basic files/folders
	createDirectory("saves");
	createDirectory("profiles");
	createDirectory("profiles/bios");
	createDirectory("profiles/game");

	LB_INFO(Frontend, "Working directory is {}", std::filesystem::current_path().string());

	// Main loop
	while(loader.run()) {
		loader.newFrame();

		ImGui::ShowDemoWindow();

		// Display Formats (2D screen or voxel rendering)
		viewport.render();
		display.render();

		// Gameboy Debug Stuff
		debug.render();
		settings.render();
		
		menubar.render();

		// render imgui
		loader.render();
	}

	gb->threadRun = false;
	// is this the best place for this?
	if (!gb->emuRun) {
		std::lock_guard lk(gb->emuM);
		gb->emuDone = false;
		gb->emuCV.notify_one();
	}
	gb->stop();
	delete gb;

	inputManager.close();

	return 0;
}
