#pragma once

#include <memory>
#include <thread>

#include "core/Config.h"
#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"
#include "PFD.h"

class SDLInterface;

namespace ui {

class FileMenu {
	Gameboy& gb;

	const bool& debugger_opened;
	SDLInterface& loader;
	std::unique_ptr<pfd::open_file> romFile = nullptr; 
	std::thread emuThread;

public:
	FileMenu(Gameboy& gb, const bool& debug, SDLInterface& loader) :
		gb(gb),
		debugger_opened(debug),
		loader(loader),
		emuThread(&Gameboy::run, &gb) {

		// todo: call this somewhere else on startup?
		gb.loadBios(config.biosPath);
	}

	~FileMenu() {
		emuThread.join();
	}

	void render();

private:
	void openFile(const std::string& filename);
};

} // namespace ui