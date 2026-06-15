#include <algorithm>
#include <filesystem>

#include "core/Gameboy.h"
#include "core/Input.h"

#include "frontend/DebugWindow.h"
#include "frontend/DisplayWindow.h"
#include "frontend/SettingsWindow.h"
#include "frontend/ViewportWindow.h"
#include "frontend/MainMenu.h"

#include "util/FileUtil.h"
#include "util/SDLInterface.h"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include "imgui_impl_sdl3.h"

struct App {
	SDLInterface* loader;

	Gameboy* gb;

	ui::DisplayWindow display;
	ui::DebugWindow debug;
	ui::SettingsWindow settings;
	ui::ViewportWindow viewport;
	ui::MainMenu menubar;

	App(SDLInterface* loader) :
		loader(loader),
		gb(new Gameboy),
		display{ *gb },
		debug{ *gb },
		settings{ *gb },
		viewport{},
		menubar{ *gb, { display.show, viewport.show, debug.show, settings.show, *loader } } {}

	~App() {
		gb->threadRun = false;
		// is this the best place for this?
		if (!gb->emuRun) {
			std::lock_guard lk(gb->emuM);
			gb->emuDone = false;
			gb->emuCV.notify_one();
		}
		gb->stop();
		delete gb;

		inputManager.close();

		delete loader;
	}

	bool run() {
		if (loader->run()) {
			loader->newFrame();
			display.render();

			ImGui::ShowDemoWindow();

			// Display Formats (2D screen or voxel rendering)
			viewport.render();

			// Gameboy Debug Stuff
			debug.render();
			settings.render();

			menubar.render();

			// render imgui
			loader->render();

			return true;
		}

		return false;
	}
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	SDLInterface* loader = new SDLInterface;
	if (!loader->init()) {
		return SDL_APP_FAILURE;
	}

	*appstate = new App(std::move(loader));

	// create basic files/folders
	createDirectory("saves");
	createDirectory("profiles");
	createDirectory("profiles/bios");
	createDirectory("profiles/raw");

  	LB_INFO(Frontend, "Working directory is {}", std::filesystem::current_path().string());
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	return (static_cast<App*>(appstate)->run()) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	App* app = static_cast<App*>(appstate);

	ImGui_ImplSDL3_ProcessEvent(event);
	inputManager.processEvent(*event);

	switch (event->type) {
		case SDL_EVENT_QUIT:
			app->loader->quit();
			break;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			if (event->window.windowID == SDL_GetWindowID(SDL_GL_GetCurrentWindow())) {
				app->loader->quit();
			}
			break;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	App* app = static_cast<App*>(appstate);

	delete app;
}
