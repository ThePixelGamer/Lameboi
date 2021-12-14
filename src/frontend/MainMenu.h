#pragma once

#include <memory>

#include "FileMenu.h"
#include "DebugMenu.h"

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

class SDLInterface;

namespace ui {

// provides access to other ui stuff to provide functionality to menu items
// todo: better name
struct MainMenuContext {
	bool& showDisplay;
	bool& showViewport;
	bool& showDebug;
	bool& showSettings;
	SDLInterface& loader;
};

class MainMenu {
	ImGuiIO& io = ImGui::GetIO();

	std::unique_ptr<FileMenu> fileMenu;
	std::unique_ptr<DebugMenu> debugMenu;

	Gameboy& gb;

	MainMenuContext context;
	bool paused = false;

	// todo: streamline?
	using second = std::chrono::duration<int>;
	using clock = std::chrono::high_resolution_clock;
	std::chrono::time_point<clock> perfTimer;
	second perf;
	u64 fps = 0;

public:
	MainMenu(Gameboy& gb, const MainMenuContext& context) :
		gb(gb),
		context(context) {
		
		fileMenu = std::make_unique<FileMenu>(gb, context.showDebug, context.loader);
		debugMenu = std::make_unique<DebugMenu>(gb, context.showDebug);
	}

	void render();

private:
	void updateFPS();
};

} // namespace ui