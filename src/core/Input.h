#pragma once

#include <functional>
#include <map>
#include <string>
#include <set>

#include <SDL3/SDL.h>

#include "util/Log.h"

struct Input {
	struct Bind {
		constexpr static SDL_Scancode invalidKey = SDL_SCANCODE_UNKNOWN;
		constexpr static SDL_GamepadButton invalidButton = SDL_GAMEPAD_BUTTON_INVALID;

		SDL_Scancode key = invalidKey;
		SDL_GamepadButton button = invalidButton;

		using Callback = std::function<void(bool)>;
		Callback callback = nullptr;

		const char* getString(bool keyboard) {
			if (keyboard) {
				if (key != invalidKey) return SDL_GetScancodeName(key);
			}
			else {
				if (button != invalidButton) return SDL_GetGamepadStringForButton(button);
			}

			return "Unbound";
		}
	};

	struct Mapping {
		std::map<std::string, Bind> binds;

		void bind(const std::string& name, Bind new_bind) {
			if (binds.find(name) != binds.end()) {
				LB_ERROR(Input, "{} bind already exists", name);
				return;
			}
			
			binds[name] = new_bind;
		}
	} mapping;

	struct Keyboard {
		using ID = SDL_KeyboardID;
	
		static const char* getName(ID id) {
			return SDL_GetKeyboardNameForID(id);
		}
	};

	struct Gamepad {
		using ID = SDL_JoystickID;

		static const char* getName(ID id) {
			if (SDL_IsGamepad(id)) {
				return SDL_GetGamepadNameForID(id);
			}
			else {
				return SDL_GetJoystickNameForID(id);
			}
		}
	};

	template<typename T>
	struct Handler {
		using ID = typename T::ID;
		
		constexpr static ID invalid = 0;

		std::set<ID> available{};
		ID active = invalid;

		void add(ID id) {
			if (id != invalid) {
				available.insert(id);
				if (active == invalid) {
					open(id);
				}
			}
		}

		void remove(ID id) {
			if (id != invalid) {
				available.erase(id);
				if (active == id) {
					close();
				}
			}
		}

		void open(ID id) {
			if (active != invalid) {
				close();
			}

			if constexpr (std::is_same_v<T, Gamepad>) {
				auto pad = SDL_OpenGamepad(id);
				if (!pad) {
					LB_WARN(Input, "Couldn't open gamepad {}: {}\n", SDL_GetGamepadNameForID(id), SDL_GetError());
					return;
				}
			}

			active = id;
		}

		void close() {
			active = invalid;

			if constexpr (std::is_same_v<T, Gamepad>) {
				if (SDL_IsGamepad(active)) {
					SDL_CloseGamepad(SDL_GetGamepadFromID(active));
				}
				else {
					SDL_CloseJoystick(SDL_GetJoystickFromID(active));
				}
			}
		}

		const char* getName() {
			if (active != invalid) {
				return T::getName(active);
			}

			return "Unknown";
		}
	};
	
	Handler<Keyboard> keyboard;
	Handler<Gamepad> gamepad;

	const char* rebind = nullptr;

	Input() = default;
	~Input() {
		gamepad.close();
	}

	void processEvent(SDL_Event& e);

	void handleKey(SDL_KeyboardEvent key);
	void handleButton(SDL_GamepadButtonEvent cbutton);
};

inline Input inputManager{};