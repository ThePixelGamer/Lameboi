#pragma once

#include "util/Common.h"
#include "util/RenderTexture.h"

class Gameboy;

namespace ui {

class BGMapWindow {
	Gameboy& gb;
		
	bool& show;
	static const size_t texWidth = 256;
	static const size_t texHeight = 256;
	std::array<u32, texWidth * texHeight> pixels;
	RenderTexture tex;

	ImVec2 oldCursor;
	bool displayOutline = false;
	int bgmap = 0;
	int tileset = 0;

public:
	BGMapWindow(Gameboy& gb, bool& show) :
		gb(gb),
		show(show),
		tex(texWidth, texHeight, pixels.data())
	{
		pixels.fill(0xFFFFFFFF);
	}

	void render();
};

} // namespace ui