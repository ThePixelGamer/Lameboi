#pragma once

#include <functional>
#include <map>

#include <SDL.h>

struct Gamepad {
	SDL_GameController* controller;
	const char* name;
};

struct Input {
	SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
	SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;

	Input() = default;
	Input(SDL_Scancode key_, SDL_GameControllerButton button_) : key(key_), button(button_) {}
	Input(SDL_Scancode key_) : key(key_) {}
	Input(SDL_GameControllerButton button_) : button(button_) {}
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

		for (int joyIndex = 0; joyIndex < SDL_NumJoysticks(); ++joyIndex) {
			addController(joyIndex);
		}
	}

	void close();
	void processEvent(SDL_Event& e);
	InputData& registerButtonHandler(const std::string& name, InputCallback callback);

	template<typename M>
	InputData& registerButtonHandler(const std::string& name, void (M::* pm)(bool), M* obj) {
		return registerButtonHandler(name, std::bind(pm, obj, std::placeholders::_1));
	}

	void addController(int device);
	void removeController(int joyIndex);

	void handleKey(SDL_KeyboardEvent key);
	void handleButton(SDL_ControllerButtonEvent cbutton);

	const std::map<int, Gamepad>& getControllers() {
		return gamepads;
	}

	const char* getControllerName(int joyIndex);
	std::string getButtonName(const char* buttonName);

	bool remapButton(const char* buttonName);
};

inline InputManager inputManager{};