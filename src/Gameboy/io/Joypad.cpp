#include "Joypad.h"

#include <fmt/printf.h>

#include "../Memory.h"

void Joypad::clean() {
	//button states
	down = up = left = right = false;
	start = select = b = a = false;

	//registers
	selectDirect = selectButton = false;
	b1 = b2 = b3 = b4 = false;
}

bool& Joypad::_mapToButton(Button button) {
	switch (button) {
		case Button::Down: return down;
		case Button::Up: return up;
		case Button::Left: return left;
		case Button::Right: return right;
		
		case Button::Start: return start;
		case Button::Select: return select;
		case Button::B: return b;
		case Button::A: return a;

		default: //change this to an assert?
			fmt::print("{} is not a valid Button", button);
			static bool null = false;
			return null;
	}
}

void Joypad::pressButton(Button button) {
	_mapToButton(button) = true;

	// set the interrupt flag when a button is pressed
	if (selectButton || selectDirect)
		interrupt.requestJoypad = true;
}

void Joypad::releaseButton(Button button) {
	_mapToButton(button) = false;
}

bool Joypad::getButtonState(Button button) {
	return _mapToButton(button);
}

u8 Joypad::read() {
	if (selectButton) {
		b4 = getButtonState(Button::Start);
		b3 = getButtonState(Button::Select);
		b2 = getButtonState(Button::B);
		b1 = getButtonState(Button::A);
	}
	else if (selectDirect) {
		b4 = getButtonState(Button::Down);
		b3 = getButtonState(Button::Up);
		b2 = getButtonState(Button::Left);
		b1 = getButtonState(Button::Right);

		if (true) { //replace with a ui setting
			if (b4 && b3)
				b4 = b3 = false;

			if (b2 && b1)
				b2 = b1 = false;
		}
	}

	//flip the bits to satisfy the 0 = button down
	return ~((selectButton << 5) | (selectDirect << 4) | 
			 (b4 << 3) | (b3 << 2) | (b2 << 1) | (b1 << 0));
}

void Joypad::write(u8 value) {
	selectDirect = !(value & 0x10);
	selectButton = !(value & 0x20);
}
