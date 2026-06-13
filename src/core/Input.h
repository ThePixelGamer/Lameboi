#pragma once

#include <functional>
#include <map>
#include <string>

#include <SDL3/SDL.h>

struct Gamepad {
	SDL_Gamepad* controller;
	const char* name;
};

struct Input {
	SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
	SDL_GamepadButton button = SDL_GAMEPAD_BUTTON_INVALID;

	Input() = default;
	Input(SDL_Scancode key_, SDL_GamepadButton button_) : key(key_), button(button_) {}
	Input(SDL_Scancode key_) : key(key_) {}
	Input(SDL_GamepadButton button_) : button(button_) {}
};

class InputManager {
	std::map<int, Gamepad> gamepads;

	using InputCallback = std::function<void(bool)>;

	struct InputData {
		Input input;
		InputCallback callback;

		void setBind(Input bind) {
			input = bind;
		}
	};

	using InputMapping = std::map<std::string, InputData>;

	// todo: implement being able to switch between maps (certain window callbacks)
	//std::map<std::string, InputMapping> inputMappings;
	InputMapping inputMapping;

public:
	bool gamepadActive;
	int selectedPad;
	Input lastInput{};

	InputManager() {
		// set pad to invalid/no controller
		gamepadActive = false;
		selectedPad = -1;

		int joyCount = 0;
		SDL_JoystickID* controllers = SDL_GetJoysticks(&joyCount);
		for (int i = 0; i < joyCount; ++i) {
			addController(controllers[i]);
		}
	}

	void close();
	void processEvent(SDL_Event& e);
	InputData& registerButtonHandler(const std::string& name, InputCallback callback);

	template<typename M>
	InputData& registerButtonHandler(const std::string& name, void (M::* pm)(bool), M* obj) {
		return registerButtonHandler(name, std::bind(pm, obj, std::placeholders::_1));
	}

	void addController(SDL_JoystickID controller_id);
	void removeController(int joyIndex);

	void handleKey(SDL_KeyboardEvent key);
	void handleButton(SDL_GamepadButtonEvent cbutton);

	const std::map<int, Gamepad>& getControllers() {
		return gamepads;
	}

	const char* getControllerName(int joyIndex);
	std::string getButtonName(const char* buttonName);

	bool remapButton(const char* buttonName);
};

inline InputManager inputManager{};