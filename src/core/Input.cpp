#include "Input.h"

// only needed for GB::Button
#include "io/Joypad.h"
#include "frontend/DisplayWindow.h"

void InputManager::close() {
	for (auto& [_, pad] : gamepads) {
		SDL_GameControllerClose(pad.controller);
	}
}

void InputManager::processEvent(SDL_Event& e) {
	switch (e.type) {
		case SDL_CONTROLLERDEVICEADDED:
			addController(e.cdevice.which);
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
			removeController(e.cdevice.which);
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			handleButton(e.cbutton);
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
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

void InputManager::addController(int device) {
	SDL_JoystickID controller_id = SDL_JoystickGetDeviceInstanceID(device);
	if (controller_id < 0) {
		LB_WARN(Input, "Couldn't get controller ID: {}\n", SDL_GetError());
		return;
	}

	Gamepad pad{};
	pad.name = SDL_JoystickNameForIndex(device);
	pad.controller = SDL_GameControllerOpen(device);

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
		SDL_GameControllerClose(it->second.controller);
		gamepads.erase(it);

		if (gamepads.empty()) {
			selectedPad = -1;
		}
	}
}

void InputManager::handleKey(SDL_KeyboardEvent key) {
	SDL_Scancode keyInput = key.keysym.scancode;
	Uint8 state = key.state;

	for (auto& [name, data] : inputMapping) {
		if (keyInput != data.input.key) {
			continue;
		}

		bool down;
		if (state == SDL_PRESSED) {
			down = true;
		}
		else if (state == SDL_RELEASED) {
			down = false;
		}
		else {
			continue;
		}

		data.callback(down);
	}

	if (state == SDL_PRESSED) {
		lastInput.key = keyInput;
	}
	else if (state == SDL_RELEASED) {
		lastInput.key = SDL_SCANCODE_UNKNOWN;
	}
}

void InputManager::handleButton(SDL_ControllerButtonEvent cbutton) {
	if (!gamepadActive || selectedPad != cbutton.which) {
		return;
	}

	auto buttonInput = static_cast<SDL_GameControllerButton>(cbutton.button);
	Uint8 state = cbutton.state;

	for (auto& [name, data] : inputMapping) {
		if (buttonInput != data.input.button) {
			continue;
		}

		bool down;
		if (state == SDL_PRESSED) {
			down = true;
		}
		else if (state == SDL_RELEASED) {
			down = false;
		}
		else {
			continue;
		}

		data.callback(down);
	}

	if (state == SDL_PRESSED) {
		lastInput.button = buttonInput;
	}
	else if (state == SDL_RELEASED) {
		lastInput.button = SDL_CONTROLLER_BUTTON_INVALID;
	}
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
		return SDL_GameControllerGetStringForButton(inputMapping[buttonName].input.button);
	}
	else {
		return SDL_GetScancodeName(inputMapping[buttonName].input.key);
	}
}

// todo: improve this
bool InputManager::remapButton(const char* buttonName) {
	if (gamepadActive) {
		if (lastInput.button == SDL_CONTROLLER_BUTTON_INVALID) {
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