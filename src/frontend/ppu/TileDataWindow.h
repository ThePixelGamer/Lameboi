#pragma once

#include "core/Gameboy.h"
#include "util/Common.h"
#include "util/ImGuiHeaders.h"
#include "util/RenderTexture.h"

namespace ui {

class TileDataWindow {
	Gameboy& gb;
		
	bool& show;
	std::array<u32, 128 * 64 * 3> pixels;
	RenderTexture tex;

	u32 zoom = 3;
	bool grid = true;

public:
	TileDataWindow(Gameboy& gb, bool& show) :
		gb(gb),
		show(show),
		tex(128, 64 * 3, pixels.data())
	{
		pixels.fill(0xFFFFFFFF);
	}

	void render();
};

} // namespace ui