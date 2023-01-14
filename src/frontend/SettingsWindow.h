#pragma once

#include <nlohmann/json.hpp>

#include "core/Config.h"
#include "core/io/Joypad.h"
#include "util/Types.h"
#include "PFD.h"

class Gameboy;

namespace ui {

class SettingsWindow {
	Gameboy& gb;

	GB::Button remapButton = GB::NumButtons;

	u8 paletteIdx;
	std::unique_ptr<pfd::open_file> paletteFile = nullptr;
	std::unique_ptr<pfd::open_file> biosFile = nullptr;

public:
	bool show = false;

	// todo: move config load/save into main.cpp?
	SettingsWindow(Gameboy& gb) :
		gb(gb) {
		paletteIdx = 0;
			
		config.load();
	}

	~SettingsWindow() {
		config.save();
	}

	void render();

private:
	void renderGeneralTab();
	void renderVideoTab();
	void renderInputTab();
	void renderGfxTab();
};

} // namespace ui