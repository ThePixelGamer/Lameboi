#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "core/Gameboy.h"

#include "frontend/DisplayWindow.h"
#include "frontend/DebugWindow.h"
#include "frontend/MainWindow.h"
#include "frontend/MainMenu.h"

#include "util/GLFWLoader.h"
#include "util/SDLHeaders.h"

int main(int, char**) {
	GLFWLoader loader;
	if (!loader.init()) {
		return 1;
	}

	SDLHandle sdl;

	auto gb = std::make_shared<Gameboy>();

	ui::DisplayWindow display(gb, loader.window);
	ui::DebugWindow debug(gb);
	ui::InputWindow input(gb);
	ui::MainWindow lameboi(gb);
	ui::MainMenu menubar(gb, debug.show, input.show);

	// create basic files/folders
	if (!std::filesystem::exists("saves")) {
		std::filesystem::create_directories("saves");
	}

	// Main loop
	while(loader.run()) {
		loader.newFrame();


		lameboi.render();
		display.render();

		// Gameboy Debug Stuff
		debug.render();

		input.render();

		menubar.render();

		//render imgui and push it the screen
		loader.render();
	}

	if (gb->running && gb->mbc) {
		gb->running = false;
		gb->mbc->close();
	}
	
	return 0;
}
