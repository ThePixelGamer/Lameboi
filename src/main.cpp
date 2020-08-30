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
#include "UI/PPUWindow.h"
#include "UI/MainWindow.h"

#include "Util/ImGuiHeaders.h"
#include "Util/SDLHeaders.h"

int main(int, char**) {
	GLFWImGui context;
	if (context.error || context.window == nullptr) {
		return 1;
	}

	auto gb = std::make_shared<Gameboy>();

	SDLInit(gb->apu);
	
	ui::DisplayWindow display(gb, context.window);
	ui::DebugWindow debug(gb);
	ui::PPUWindow ppu(gb); 
	ui::InputWindow input(gb);
	ui::MainWindow menu(gb, debug.show, ppu.show, input.show);

	if (!std::filesystem::exists("saves")) {
		std::filesystem::create_directories("saves");
	}

	// Main loop
	while(context.NewFrame()) {
		menu.render();
		display.render();

		// Gameboy Stuff
		ppu.render();
		debug.render();

		input.render();

		context.Render();
	}

	if (gb->running && gb->mbc) {
		gb->running = false;
		gb->mbc->close();
	}

	SDLUninit();
	
	return 0;
}
