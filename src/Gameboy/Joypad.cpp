#include "Joypad.h"

#include "Memory.h"

void Joypad::pressButton(Button button) {
	switch (button) {
		case Button::Down: {
			down = true;
		} break;
		case Button::Up: {
			up = true;
		} break;
		case Button::Left: {
			left = true;
		} break;
		case Button::Right: {
			right = true;
		} break;
		case Button::Start: {
			start = true;
		} break;
		case Button::Select: {
			select = true;
		} break;
		case Button::B: {
			b = true;
		} break;
		case Button::A: {
			a = true;
		} break;
	}

	// set the interrupt flag when a button is pressed
	if (mem.joypad.selectButton == 0 || mem.joypad.selectDirect == 0)
		mem.IF.joypad = 1;
}

void Joypad::releaseButton(Button button) {
	switch (button) {
		case Button::Down: {
			down = false;
		} break;
		case Button::Up: {
			up = false;
		} break;
		case Button::Left: {
			left = false;
		} break;
		case Button::Right: {
			right = false;
		} break;
		case Button::Start: {
			start = false;
		} break;
		case Button::Select: {
			select = false;
		} break;
		case Button::B: {
			b = false;
		} break;
		case Button::A: {
			a = false;
		} break;
	}
}

bool Joypad::getButtonState(Button button) {
	switch (button) {
		case Button::Down: {
			return down;
		}
		case Button::Up: {
			return up;
		}
		case Button::Left: {
			return left;
		}
		case Button::Right: {
			return right;
		}
		case Button::Start: {
			return start;
		}
		case Button::Select: {
			return select;
		}
		case Button::B: {
			return b;
		}
		case Button::A: {
			return a;
		}
	}

	return false;
}