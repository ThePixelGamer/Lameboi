#pragma once

#include "util/Common.h"
#include "frontend/widgets/Image.h"

class Gameboy;

namespace ui {

class BGMapWindow {
	Gameboy& gb;
		
	Image<256, 256> bgmapTex{};

	ImVec2 oldCursor;
	bool displayOutline = false;
	int bgmap = 0;
	int tileset = 0;

	// todo: move
	// todo: support more than a linear selection
	Pos2 initialClick = { 0, 0 };
	Pos2 selectionMin = { 0, 0 }, selectionMax = { 0, 0 };
	bool selected = false;

	Image<32 * 8, 32 * 8> dump{};
	std::string dumpFile = "";

public:
	bool show = false;

	BGMapWindow(Gameboy& gb) :
		gb(gb)
	{
		bgmapTex.data().fill(0xFF);
	}

	void render();

private:
	void handleClick(u32 x, u32 y);
	void drawExtra(const ImVec2& topleft, const ImVec2& bottomright, float mult);
};

} // namespace ui