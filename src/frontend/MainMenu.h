#pragma once

#include <memory>
#include <PFD.h>

class Gameboy;
struct UI;

namespace ui {

class MainMenu {
	// file menu
	std::unique_ptr<pfd::open_file> romFile = nullptr;
	bool showDemoWindow = false;

	UI& app;
	Gameboy& gb;

public:
	MainMenu(UI& app);

	void render();

private:
	void renderFile();
	void renderGameboy();
	void renderDebug();
};

} // namespace ui