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

	std::unique_ptr<Gameboy> gb;

	std::unique_ptr<ui::DisplayWindow> display;
	std::unique_ptr<ui::DebugWindow> debug;
	std::unique_ptr<ui::SettingsWindow> settings;
	std::unique_ptr<ui::ViewportWindow> viewport;
	std::unique_ptr<ui::MainMenu> menubar;

	std::unique_ptr<ui::BGMapWindow> bgmapWindow;
	std::unique_ptr<ui::TileDataWindow> tileDataWindow;
	std::unique_ptr<ui::OAMWindow> oamWindow;

	App() = default;

	~App() {
		gb->threadRun = false;
		// is this the best place for this?
		if (!gb->emuRun) {
			std::lock_guard lk(gb->emuM);
			gb->emuDone = false;
			gb->emuCV.notify_one();
		}
		gb->stop();

		inputManager.close();
	}

	void init() {
		gb = std::make_unique<Gameboy>();
		display = std::make_unique<ui::DisplayWindow>(*this);
		debug = std::make_unique<ui::DebugWindow>(*gb);
		settings = std::make_unique<ui::SettingsWindow>(*gb);
		viewport = std::make_unique<ui::ViewportWindow>();
		menubar = std::make_unique<ui::MainMenu>(*gb, *this);

		bgmapWindow = std::make_unique<ui::BGMapWindow>(*gb);
		tileDataWindow = std::make_unique<ui::TileDataWindow>(*gb);
		oamWindow = std::make_unique<ui::OAMWindow>(*gb);
	}

	void render() {
		display->render();
		menubar->render();

		// Display Formats (2D screen or voxel rendering)
		viewport->render();

		// Gameboy Debug Stuff
		debug->render();
		settings->render();
		bgmapWindow->render();
		tileDataWindow->render();
		oamWindow->render();
	}
};