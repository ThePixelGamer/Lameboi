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

	// todo: move
	// todo: support more than a linear selection
	Pos2 initialClick = { 0, 0 };
	Pos2 selectionMin = { 0, 0 }, selectionMax = { 0, 0 };
	bool selected = false;

	std::array<u32, 32 * 8 * 32 * 8> dumpPixels;
	RenderTexture dumpPreview;

public:
	BGMapWindow(Gameboy& gb, bool& show) :
		gb(gb),
		show(show),
		tex(texWidth, texHeight, pixels.data()),
		dumpPreview(32 * 8, 32 * 8, dumpPixels.data())
	{
		pixels.fill(0xFFFFFFFF);
	}

	void render();

private:
	void handleClick(u32 x, u32 y);
	static void drawExtra(void* extraData, const ImVec2& topleft, const ImVec2& bottomright, float mult);
};

} // namespace ui