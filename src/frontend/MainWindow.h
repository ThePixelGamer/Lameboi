#pragma once

#include "DebugWindow.h"
#include "DisplayWindow.h"
#include "SettingsWindow.h"
#include "ViewportWindow.h"
#include "MainMenu.h"
#include "ppu/BGMapWindow.h"
#include "ppu/OAMWindow.h"
#include "ppu/TileDataWindow.h"

class Gameboy;

struct UI {
	bool requestExit = false;

	Gameboy& gb;

	ui::DisplayWindow display;
	ui::DebugWindow debug;
	ui::SettingsWindow settings;
	ui::ViewportWindow viewport;
	ui::MainMenu menubar;

	ui::BGMapWindow bgmapWindow;
	ui::TileDataWindow tileDataWindow;
	ui::OAMWindow oamWindow;

	UI(Gameboy& gb) :
		gb(gb),
		display(*this),
		debug(gb),
		settings(gb),
		viewport(),
		menubar(*this),
		bgmapWindow(gb),
		tileDataWindow(gb),
		oamWindow(gb) {

	}

	bool render();
};
