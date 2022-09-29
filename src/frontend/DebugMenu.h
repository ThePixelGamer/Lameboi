#pragma once

#include "PPUMenu.h"

class Debugger;

namespace ui {

class DebugMenu {
	Debugger& debug;

	std::unique_ptr<PPUMenu> ppuMenu;

	bool& showDebug;

public:
	DebugMenu(Gameboy& gb, bool& debug);

	void render();
};

} // namespace ui