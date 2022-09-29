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
	
	bool valid = false;
	bool second = false;
	u32 x1 = 0, y1 = 0;
	u32 x2 = 0, y2 = 0;
	u32 tiles = 0;
	int height = 1, minHeight = 1;
	std::array<u32, 32 * 8 * 32 * 8> dumpPixels;
	RenderTexture dumpPreview;

	u32 zoom = 3;
	bool grid = true;

public:
	TileDataWindow(Gameboy& gb, bool& show) :
		gb(gb),
		show(show),
		tex(128, 64 * 3, pixels.data()),
		dumpPreview(32 * 8, 32 * 8, dumpPixels.data())
	{
		pixels.fill(0xFFFFFFFF);
	}

	void render();
};

} // namespace ui