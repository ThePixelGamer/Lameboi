#pragma once

#include <array>

#include "frontend/widgets/Image.h"
#include "util/Types.h"

class Gameboy;

namespace ui {

class TileDataWindow {
	Gameboy& gb;
		
	Image<128, 64 * 3> tilemap{};
	
	bool valid = false;
	bool second = false;
	u32 x1 = 0, y1 = 0;
	u32 x2 = 0, y2 = 0;
	u32 tiles = 0;
	int height = 1, minHeight = 1;
	Image<32 * 8, 32 * 8> dump{};

	u32 zoom = 3;
	bool grid = true;

public:
	bool show = false;

	TileDataWindow(Gameboy& gb) :
		gb(gb)
	{
		dump.data().fill(0xFF);
	}

	void render();
};

} // namespace ui