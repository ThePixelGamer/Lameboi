#pragma once

#include "util/Common.h"
#include "util/Color.h"
#include "util/ImGuiHeaders.h"
#include "util/RenderTexture.h"

class Gameboy;

namespace ui {

class OAMWindow {
	Gameboy& gb;
		
	bool& show;
	std::array<u32, 64 * 40> pixels;
	Color invisColor;
	RenderTexture tex;

	u32 zoom = 3;
	bool grid = true;
	u32 step = 1;

public:
	OAMWindow(Gameboy& gb, bool& show) :
		gb(gb),
		show(show),
		tex(64, 40, pixels.data())
	{
		pixels.fill(0xFFFFFFFF);
	}

	void render();
};

} // namespace ui