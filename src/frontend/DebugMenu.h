#pragma once

#include "PPUMenu.h"

class Gameboy;

namespace ui {

class DebugMenu {
	Gameboy& gb;

	std::unique_ptr<PPUMenu> ppuMenu;

	bool& showDebug;

public:
	DebugMenu(Gameboy& gb, bool& debug) :
		gb(gb),
		showDebug(debug) {

		ppuMenu = std::make_unique<PPUMenu>(gb);
	}

	void render();
};

} // namespace ui