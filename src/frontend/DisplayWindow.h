#pragma once

#include <array>

#include "util/ImGuiHeaders.h"
#include "util/RenderTexture.h"

class App;

namespace ui {

class DisplayWindow {
	App& context;

	RenderTexture display;
	
	static constexpr size_t displayWidth = 160;
	static constexpr size_t displayHeight = 144;
	std::array<u32, displayWidth * displayHeight> displayBuf{};

	ImVec2 oldCursor;
	// todo: update with the window math
	u32 zoom = 3;

public:
	bool show = true;
	bool integerScaling = false;
	bool useCG = false;
	// maybe move this into Input?
	static inline bool focused = false;

	DisplayWindow(App& context) :
		context(context),
		display(displayWidth, displayHeight, displayBuf.data()) {}

	void render();

private:
	void updateBuffer();
};

} // namespace ui 