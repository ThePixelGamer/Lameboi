#pragma once

#include <array>
#include <string>
#include <map>

#include "util/Color.h"
#include "util/Settings.h"
#include "util/Types.h"

struct Config {
	// PPU
	Setting<std::string> currentPalette{ "default" };
	std::map<std::string, Palette> paletteProfiles;

	// APU
	Setting<float> volume{ 0.1f };
	Setting<bool> audioSync{ true };
};

inline Config config;