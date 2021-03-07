#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "Gameboy/Gameboy.h"

#include "UI/DisplayWindow.h"
#include "UI/DebugWindow.h"
#include "UI/InputWindow.h"
#include "UI/MainWindow.h"
#include "UI/MainMenu.h"

#include "Util/ImGuiHeaders.h"
#include "Util/SDLHeaders.h"

int main(int, char**) {
	GLFWImGui context;
	if (context.error || context.window == nullptr) {
		return 1;
	}

	SDLHandle sdl;

	auto gb = std::make_shared<Gameboy>();

	ui::DisplayWindow display(gb, context.window);
	ui::DebugWindow debug(gb);
	ui::InputWindow input(gb);
	ui::MainWindow lameboi(gb);
	ui::MainMenu menubar(gb, debug.show, input.show);

	if (!std::filesystem::exists("saves")) {
		std::filesystem::create_directories("saves");
	}

	// Main loop
	while(context.NewFrame()) {


		lameboi.render();
		display.render();

		// Gameboy Debug Stuff
		debug.render();

		input.render();

		menubar.render();

		//render imgui and push it the screen
		context.Render();
	}

	if (gb->running && gb->mbc) {
		gb->running = false;
		gb->mbc->close();
	}
	
	return 0;
}
