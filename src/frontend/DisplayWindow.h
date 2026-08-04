#pragma once

#include <chrono>

#include "widgets/Image.h"

struct UI;

namespace ui {

class DisplayWindow {
	UI& context;

	using Display = Image<160, 144>;
	Display display;

	ImVec2 oldCursor;
	// todo: update with the window math
	u32 zoom = 3;

	// todo: streamline?
	using clock = std::chrono::high_resolution_clock;
	clock::time_point perfTimer = clock::now();
	u16 fps = 0;
	u64 instrCount = 0;

public:
	bool show = true;
	bool integerScaling = false;
	bool useCG = false;
	// maybe move this into Input?
	static inline bool focused = false;

	DisplayWindow(UI& context);
	void render();

private:
	void updateBuffer();
};

} // namespace ui 
