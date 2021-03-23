#pragma once

#include "../Interrupt.h"
#include "util/Types.h"

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
	Interrupt& interrupt;

	//button states
	bool down, up, left, right;
	bool start, select, b, a;

	//registers
	bool selectDirect; 
	bool selectButton;
	bool b1, b2, b3, b4;

public:
	Joypad(Interrupt& interrupt) : interrupt(interrupt) {
		clean();
	}

	void clean();

	void pressButton(Button button);
	void releaseButton(Button button);

	bool getButtonState(Button button);

	u8 read();
	void write(u8);

private:
	bool& _mapToButton(Button button);
};