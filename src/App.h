#pragma once

#include "core/Gameboy.h"
#include "core/Input.h"

#include "frontend/DebugWindow.h"
#include "frontend/DisplayWindow.h"
#include "frontend/SettingsWindow.h"
#include "frontend/ViewportWindow.h"
#include "frontend/MainMenu.h"
#include "frontend/ppu/BGMapWindow.h"
#include "frontend/ppu/OAMWindow.h"
#include "frontend/ppu/TileDataWindow.h"

struct App {
	bool requestExit = false;

	Gameboy gb;
	std::thread emuThread;

	ui::DisplayWindow display;
	ui::DebugWindow debug;
	ui::SettingsWindow settings;
	ui::ViewportWindow viewport;
	ui::MainMenu menubar;

	ui::BGMapWindow bgmapWindow;
	ui::TileDataWindow tileDataWindow;
	ui::OAMWindow oamWindow;

	App() : 
		display(*this),
		debug(gb),
		settings(gb),
		viewport(),
		menubar(*this),
		bgmapWindow(gb),
		tileDataWindow(gb),
		oamWindow(gb),

		emuThread(&Gameboy::thread, &gb)
	{
		gb.loadBios(config.biosPath);
	}

	~App() {
		gb.exit();
		emuThread.join();

		inputManager.close();
	}

	void render() {
		display.render();
		menubar.render();

		// Display Formats (2D screen or voxel rendering)
		viewport.render();

		// Gameboy Debug Stuff
		debug.render();
		settings.render();
		bgmapWindow.render();
		tileDataWindow.render();
		oamWindow.render();
	}
};