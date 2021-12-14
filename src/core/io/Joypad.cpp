#include "Joypad.h"

#include <fmt/printf.h>

#include "core/Config.h"
#include "core/Input.h"
#include "core/Interrupt.h"
#include "core/Memory.h"

template<GB::Button Button>
void Joypad::updateButton(bool down) {
	if (down) {
		pressButton(Button);
	}
	else {
		releaseButton(Button);
	}
}

Joypad::Joypad(Interrupt& interrupt) : interrupt(interrupt) {
	clean();

	// should I change this?
	inputManager.registerButtonHandler("Up", &Joypad::updateButton<GB::Up>, this)
		.setBind({ SDL_SCANCODE_W, SDL_CONTROLLER_BUTTON_DPAD_UP });
	inputManager.registerButtonHandler("Down", &Joypad::updateButton<GB::Down>, this)
		.setBind({ SDL_SCANCODE_S, SDL_CONTROLLER_BUTTON_DPAD_DOWN });
	inputManager.registerButtonHandler("Left", &Joypad::updateButton<GB::Left>, this)
		.setBind({ SDL_SCANCODE_A, SDL_CONTROLLER_BUTTON_DPAD_LEFT });
	inputManager.registerButtonHandler("Right", &Joypad::updateButton<GB::Right>, this)
		.setBind({ SDL_SCANCODE_D, SDL_CONTROLLER_BUTTON_DPAD_RIGHT });

	inputManager.registerButtonHandler("Start", &Joypad::updateButton<GB::Start>, this)
		.setBind({ SDL_SCANCODE_RETURN, SDL_CONTROLLER_BUTTON_START });
	inputManager.registerButtonHandler("Select", &Joypad::updateButton<GB::Select>, this)
		.setBind({ SDL_SCANCODE_E, SDL_CONTROLLER_BUTTON_BACK });
	inputManager.registerButtonHandler("B", &Joypad::updateButton<GB::B>, this)
		.setBind({ SDL_SCANCODE_SEMICOLON, SDL_CONTROLLER_BUTTON_X });
	inputManager.registerButtonHandler("A", &Joypad::updateButton<GB::A>, this)
		.setBind({ SDL_SCANCODE_APOSTROPHE, SDL_CONTROLLER_BUTTON_A });
}

void Joypad::clean() {
	//button states
	buttonStates.fill(false);

	//registers
	selectDirect = selectButton = false;
	b1 = b2 = b3 = b4 = false;
}

bool& Joypad::_mapToButton(GB::Button button) {
	if (button == GB::NumButtons) {
		fmt::print("{} is not a valid GB::Button", button);
		static bool null = false;
		return null;
	}
	else {
		return buttonStates[button];
	}
}

void Joypad::pressButton(GB::Button button) {
	_mapToButton(button) = true;

	// set the interrupt flag when a button is pressed
	if (selectButton || selectDirect)
		interrupt.requestJoypad = true;
}

void Joypad::releaseButton(GB::Button button) {
	_mapToButton(button) = false;
}

bool Joypad::getButtonState(GB::Button button) {
	return _mapToButton(button);
}

u8 Joypad::read() {
	if (selectButton) {
		b4 = getButtonState(GB::Start);
		b3 = getButtonState(GB::Select);
		b2 = getButtonState(GB::B);
		b1 = getButtonState(GB::A);
	}
	else if (selectDirect) {
		b4 = getButtonState(GB::Down);
		b3 = getButtonState(GB::Up);
		b2 = getButtonState(GB::Left);
		b1 = getButtonState(GB::Right);

		if (!config.oppositeDir) { // allow opposite direction presses at the same time
			if (b4 && b3)
				b4 = b3 = false;

			if (b2 && b1)
				b2 = b1 = false;
		}
	}

	// flip the bits to satisfy the 0 = button down
	return ~((selectButton << 5) | (selectDirect << 4) | 
			 (b4 << 3) | (b3 << 2) | (b2 << 1) | (b1 << 0));
}

void Joypad::write(u8 value) {
	selectDirect = !(value & 0x10);
	selectButton = !(value & 0x20);
}
