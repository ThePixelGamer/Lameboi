#pragma once

#include "Util/Types.h"

class Memory;

enum class Button : u8 {
	Up,
	Down,
	Left,
	Right,
	Start,
	Select,
	B,
	A
};

class Joypad {
	Memory& mem;

	//button states
	bool down = false;
	bool up = false;
	bool left = false;
	bool right = false;
	bool start = false;
	bool select = false;
	bool b = false;
	bool a = false;

public:
	Joypad(Memory& mem) : mem(mem) {}

	void pressButton(Button button);
	void releaseButton(Button button);

	bool getButtonState(Button button);
};