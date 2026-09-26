#pragma once

#include "DebugWindow.h"
#include "DisplayWindow.h"
#include "SettingsWindow.h"
#include "MainMenu.h"

#include "APUWindow.h"

class Gameboy;

struct UI {
	bool requestExit = false;

	Gameboy& gb;

	ui::DisplayWindow display;
	ui::DebugWindow debug;
	ui::SettingsWindow settings;
	ui::MainMenu menubar;

	ui::APUWindow apuWindow;

	UI(Gameboy& gb) :
		gb(gb),
		display(*this),
		debug(gb),
		settings(gb),
		menubar(*this),
		apuWindow(gb) {

	}

	bool render();
};
