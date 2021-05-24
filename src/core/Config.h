#pragma once

#include <array>
#include <map>
#include <string>

#include "util/Color.h"
#include "util/Settings.h"
#include "util/Types.h"

using PaletteProfile = std::map<std::string, Palette>;

struct Config {
	// General
	Setting<bool> inputOverlay{ true };
	Setting<std::string> biosPath{ "dmg_boot.bin" }; // should resolve to a full directory when saving

	// Video
	Setting<std::string> currentPalette{ "default" };
	Setting<PaletteProfile> paletteProfiles{ {
		{ "default", { 0x9bbc0f, 0x8bac0f, 0x306230, 0x0f380f } },
		{ "hollow",  { 0xfafbf6, 0xc6b7be, 0x565a75, 0x0f0f1b } } // https://lospec.com/palette-list/hollow
	} };

	// Audio
	Setting<float> volume{ 0.1f };
	Setting<bool> audioSync{ true };

	// Input
	Setting<bool> oppositeDir{ false };

	void load();
	void save();
};

inline Config config;