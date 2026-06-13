#include "Input.h"

// only needed for GB::Button
#include "io/Joypad.h"
#include "frontend/DisplayWindow.h"

void InputManager::close() {
	for (auto& [_, pad] : gamepads) {
		SDL_CloseGamepad(pad.controller);
	}
}

void InputManager::processEvent(SDL_Event& e) {
	switch (e.type) {
		case SDL_EVENT_GAMEPAD_ADDED:
			addController(e.cdevice.which);
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			removeController(e.cdevice.which);
			break;

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


InputManager::InputData& InputManager::registerButtonHandler(const std::string& name, InputCallback callback) {
	if (inputMapping.find(name) != inputMapping.end()) {
		LB_ERROR(Input, "{} already exists in the input map.", name);

		// make sure they don't modify the one in the map
		static InputData invalid{ {}, nullptr };
		return invalid;
	}

	// todo: handle if inserted.second == false?
	return inputMapping.emplace(name, InputData{ {}, callback }).first->second;
}

void InputManager::addController(SDL_JoystickID controller_id) {
	if (controller_id == 0) {
		LB_WARN(Input, "Invalid controller ID: {}\n", SDL_GetError());
		return;
	}

	Gamepad pad{};
	pad.name = SDL_GetJoystickNameForID(controller_id);
	pad.controller = SDL_OpenGamepad(controller_id);

	if (!pad.controller) {
		LB_WARN(Input, "Couldn't open gamecontroller {}: {}\n", pad.name, SDL_GetError());
		return;
	}

	gamepads[controller_id] = pad;

	if (selectedPad == -1) {
		selectedPad = controller_id;
	}
}

void InputManager::removeController(int joyIndex) {
	auto it = gamepads.find(joyIndex);
	if (it != gamepads.end()) {
		SDL_CloseGamepad(it->second.controller);
		gamepads.erase(it);

		if (gamepads.empty()) {
			selectedPad = -1;
		}
	}
}

void InputManager::handleKey(SDL_KeyboardEvent key) {
	SDL_Scancode keyInput = key.scancode;

	for (auto& [name, data] : inputMapping) {
		if (keyInput != data.input.key) {
			continue;
		}

		data.callback(key.down);
	}

	lastInput.key = (key.down) ? keyInput : SDL_SCANCODE_UNKNOWN;
}

void InputManager::handleButton(SDL_GamepadButtonEvent cbutton) {
	if (!gamepadActive || selectedPad != cbutton.which) {
		return;
	}

	auto buttonInput = static_cast<SDL_GamepadButton>(cbutton.button);

	for (auto& [name, data] : inputMapping) {
		if (buttonInput != data.input.button) {
			continue;
		}

		data.callback(cbutton.down);
	}

	lastInput.button = (cbutton.down) ? buttonInput : SDL_GAMEPAD_BUTTON_INVALID;
}

const char* InputManager::getControllerName(int joyIndex) {
	if (!gamepads.empty()) {
		return gamepads[joyIndex].name;
	}
	else {
		return "No controller";
	}
}

std::string InputManager::getButtonName(const char* buttonName) {
	auto it = inputMapping.find(buttonName);
	if (it == inputMapping.end()) {
		return "Unknown";
	}

	if (gamepadActive) {
		return SDL_GetGamepadStringForButton(inputMapping[buttonName].input.button);
	}
	else {
		return SDL_GetScancodeName(inputMapping[buttonName].input.key);
	}
}

// todo: improve this
bool InputManager::remapButton(const char* buttonName) {
	if (gamepadActive) {
		if (lastInput.button == SDL_GAMEPAD_BUTTON_INVALID) {
			return false;
		}

		auto it = inputMapping.find(buttonName);
		if (it != inputMapping.end()) {
			it->second.input.button = lastInput.button;
			return true;
		}
	}
	else {
		if (lastInput.key == SDL_SCANCODE_UNKNOWN) {
			return false;
		}

		auto it = inputMapping.find(buttonName);
		if (it != inputMapping.end()) {
			it->second.input.key = lastInput.key;
			return true;
		}
	}

	return false;
}