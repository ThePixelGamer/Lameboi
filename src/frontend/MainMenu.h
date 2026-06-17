#pragma once

#include <memory>

#include "core/Gameboy.h"
#include <PFD.h>

struct App;

namespace ui {

class MainMenu {
	// file menu
	std::unique_ptr<pfd::open_file> romFile = nullptr;
	std::thread emuThread;

	Gameboy& gb;

	App& context;
	bool paused = false;

	// todo: streamline?
	using second = std::chrono::duration<int>;
	using clock = std::chrono::high_resolution_clock;
	std::chrono::time_point<clock> perfTimer;
	second perf;
	u64 fps = 0;

public:
	MainMenu(Gameboy& gb, App& context);

	~MainMenu() {
		emuThread.join();
	}

	void render();

private:
	void renderFile();
	void renderGameboy();
	void renderDebug();

	void updateFPS();
};

} // namespace ui