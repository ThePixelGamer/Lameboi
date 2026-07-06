#include "Input.h"


void Input::processEvent(SDL_Event& e) {
	switch (e.type) {
		case SDL_EVENT_KEYBOARD_ADDED: keyboard.add(e.kdevice.which); break;
		case SDL_EVENT_KEYBOARD_REMOVED: keyboard.remove(e.kdevice.which); break;

		case SDL_EVENT_GAMEPAD_ADDED: gamepad.add(e.kdevice.which); break;
		case SDL_EVENT_GAMEPAD_REMOVED: gamepad.remove(e.kdevice.which); break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			handleButton(e.gbutton);
			break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			handleKey(e.key);
			break;

		default: break;
	}
}

void Input::handleKey(SDL_KeyboardEvent key) {
	SDL_Scancode keyInput = key.scancode;

	for (auto& [name, bind] : mapping.binds) {
		if (bind.key == keyInput) {
			bind.callback(key.down);
		}

		if (key.down && rebind && name == rebind) {
			bind.key = keyInput;
			rebind = nullptr;
		}
	}
}

void Input::handleButton(SDL_GamepadButtonEvent cbutton) {
	if (gamepad.active != cbutton.which) {
		return;
	}

	auto buttonInput = static_cast<SDL_GamepadButton>(cbutton.button);

	for (auto& [name, bind] : mapping.binds) {
		if (bind.button == buttonInput) {
			bind.callback(cbutton.down);
		}

		if (cbutton.down && rebind && name == rebind) {
			bind.button = buttonInput;
			rebind = nullptr;
		}
	}
}