#include "Joypad.h"

#include <fmt/printf.h>

#include "core/Config.h"
#include "core/Input.h"
#include "core/Interrupt.h"
#include "core/Memory.h"

void Joypad::updateButton(Button button, bool down) {
	buttonStates[button] = down;

	if (down) {
		// set the interrupt flag when a button is pressed
		if (selectButton || selectDirect) interrupt.request.joypad = true;

		// dpad button, release opposite direction button if config is set
		if (!(button & 0x4) && !config.oppositeDir) {
			buttonStates[button ^ 0x1] = false;
		}
	}
}

Joypad::Joypad(Memory& bus, Interrupt& interrupt) : interrupt(interrupt) {
	clean();

	bus.register_io(0x0, bus.register_bus(
		[](void* d, addr) { return static_cast<Joypad*>(d)->read(); },
		[](void* d, addr, u8 v) { return static_cast<Joypad*>(d)->write(v); },
		this
	));

	// should I change this?
	auto registerButton = [&](Button button, SDL_Scancode defaultKey, SDL_GamepadButton defaultButton) {
		Input::Bind bind{};
		bind.key = defaultKey;
		bind.button = defaultButton;
		using namespace std::placeholders;
		bind.callback = std::bind(&Joypad::updateButton, this, button, _1);
		inputManager.mapping.bind(names[button], bind);
	};

	registerButton(Down, SDL_SCANCODE_S, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
	registerButton(Up, SDL_SCANCODE_W, SDL_GAMEPAD_BUTTON_DPAD_UP);
	registerButton(Left, SDL_SCANCODE_A, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
	registerButton(Right, SDL_SCANCODE_D, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

	registerButton(Start, SDL_SCANCODE_RETURN, SDL_GAMEPAD_BUTTON_START);
	registerButton(Select, SDL_SCANCODE_E, SDL_GAMEPAD_BUTTON_BACK);
	registerButton(B, SDL_SCANCODE_SEMICOLON, SDL_GAMEPAD_BUTTON_WEST);
	registerButton(A, SDL_SCANCODE_APOSTROPHE, SDL_GAMEPAD_BUTTON_SOUTH);
}

void Joypad::clean() {
	//button states
	buttonStates.fill(false);

	//registers
	selectDirect = selectButton = false;
}

u8 Joypad::read() {
	bool b[0x4]{};

	if (selectDirect) {
		b[3] |= buttonStates[Down];
		b[2] |= buttonStates[Up];
		b[1] |= buttonStates[Left];
		b[0] |= buttonStates[Right];
	}

	if (selectButton) {
		b[3] |= buttonStates[Start];
		b[2] |= buttonStates[Select];
		b[1] |= buttonStates[B];
		b[0] |= buttonStates[A];
	}

	// flip the bits to satisfy the 0 = button down
	return ~((b[3] << 3) | (b[2] << 2) | (b[1] << 1) | (b[0] << 0));
}

void Joypad::write(u8 value) {
	selectDirect = !(value & 0x10);
	selectButton = !(value & 0x20);
}
