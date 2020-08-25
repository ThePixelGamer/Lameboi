#pragma once

#include <SDL.h>
#include <SDL_audio.h>

#include "Gameboy/APU.h"

inline void SDLInit(APU& apu) {
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec spec;
	spec.freq		= APU::frequency;
	spec.format		= AUDIO_S16SYS;
	spec.channels	= 2;
	spec.samples	= APU::samples;
	spec.callback	= &sound_mix;
	spec.userdata	= &apu;

	SDL_OpenAudio(&spec, nullptr);
	SDL_PauseAudio(0);
}

inline void SDLUninit() {
	SDL_CloseAudio();
	SDL_Quit();
}