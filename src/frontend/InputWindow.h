#pragma once

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"

namespace ui {
	class InputWindow {
		std::shared_ptr<Gameboy> gb;
		static inline int currentKey = GLFW_KEY_UNKNOWN;

		const char* remapKeyName = "";

	public:
		bool show = false;
		static inline int up	 = GLFW_KEY_W;
		static inline int down	 = GLFW_KEY_S;
		static inline int left	 = GLFW_KEY_A;
		static inline int right  = GLFW_KEY_D;
		static inline int b		 = GLFW_KEY_SEMICOLON;
		static inline int a		 = GLFW_KEY_APOSTROPHE;
		static inline int start  = GLFW_KEY_ENTER;
		static inline int select = GLFW_KEY_E;

		InputWindow(std::shared_ptr<Gameboy> gb) :
			gb(gb)
		{}

		void render() {
			if (show) {
				ImGui::Begin("Inputs", &show);

				auto inputRemap = [this](const char* name, int& key) {
					if (ImGui::Button(name)) {
						remapKeyName = name;
					}

					//the pointer compare works here because we're assigning using the passed in name
					if (currentKey != GLFW_KEY_UNKNOWN && remapKeyName == name) {
						key = currentKey;
						remapKeyName = "";
					}

					ImGui::SameLine();

					std::string modified = " ";

					auto keyName = glfwGetKeyName(key, glfwGetKeyScancode(key));
					if (keyName == nullptr) {
						switch (key) {
							case GLFW_KEY_ENTER: modified += "Enter"; break;
							default: modified += "Unknown"; break;
						}
					}
					else {
						// this is awful : )
						modified += std::toupper(*keyName);
						modified += (keyName + 1);
					}

					ImGui::Text(modified.c_str());
				};

				inputRemap("Up", up);
				inputRemap("Down", down);
				inputRemap("Left", left);
				inputRemap("Right", right);
				inputRemap("B", b);
				inputRemap("A", a);
				inputRemap("Start", start);
				inputRemap("Select", select);

				ImGui::End();
			}
		}

		static void KeyMapHelper(GLFWwindow* window, int key, int scancode, int action, int mods) {
			if (action == GLFW_PRESS) {
				currentKey = key;
			}
			else {
				currentKey = GLFW_KEY_UNKNOWN;
			}
		}
	};
}