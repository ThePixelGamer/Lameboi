#pragma once

class Gameboy;

namespace ui {

class APUWindow {
private:
	Gameboy& gb;	

public:
	bool show = false;

	APUWindow(Gameboy& gb) : gb(gb) {

	}

	void render();
};

}
