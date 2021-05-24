#pragma once

#include <SDL.h>

//may need a better name :P
class SDLHandle {
public:
	SDLHandle() {
		SDL_Init(SDL_INIT_AUDIO);
	}

	~SDLHandle() {
		SDL_Quit();
	}
};