#pragma once

#include <chrono>

#include "render/Scene.h"
#include "widgets/Image.h"

struct UI;
struct Framebuffer;

namespace ui {

class DisplayWindow {
	UI& context;

	using Display = Image<160, 144>;
	Display display;
	Scene scene;
	bool inFocus = false; 
	bool avoidReset = false;

	ImVec2 oldCursor;
	// todo: update with the window math
	u32 zoom = 3;

	// todo: streamline?
	using clock = std::chrono::high_resolution_clock;
	clock::time_point perfTimer = clock::now();
	u16 fps = 0;
	u64 instrCount = 0;

public:
	enum class Mode {
		Display = 1 << 0,
		Scene = 1 << 1,
		SideBySide = Display | Scene | 0 << 2,
		DisplayOnTop = Display | Scene | 1 << 2,
		SceneOnTop = Display | Scene | 2 << 2,
	} mode = Mode::SideBySide;

	bool fullscreen = false;
	bool integerScaling = false;
	bool useCG = false;
	// maybe move this into Input?
	static inline bool focused = false;

	DisplayWindow(UI& context);
	void render();

private:
	void contextMenu();
	void updateBuffer();

	void renderCG(Framebuffer& buffer);
	void renderScene(Framebuffer& buffer);

	void captureMouse(bool hasMouse);
};

} // namespace ui 
