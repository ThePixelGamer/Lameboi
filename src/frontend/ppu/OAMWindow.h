#pragma once

#include "util/Common.h"
#include "util/Color.h"
#include "frontend/widgets/Image.h"

class Gameboy;

namespace ui {

class OAMWindow {
	Gameboy& gb;
		
	ImVec4 invisColor = ImColor(IM_COL32_WHITE);
	Image<64, 40> tex{};

	u32 zoom = 3;
	bool grid = true;
	u32 step = 1;

public:
	bool show = false;

	OAMWindow(Gameboy& gb) :
		gb(gb)
	{
		tex.data().fill(0xFF);
	}

	void render();
};

} // namespace ui
