#pragma once

#include <SDL.h>

class SDLInterface {
	SDL_Window* window = nullptr; 
	SDL_GLContext gl_context = nullptr;
	bool done = false;

public:
	~SDLInterface();

	bool init();
	void quit();
	bool run();
	void newFrame();
	void render();
};