#pragma once

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

namespace ui {
	class DisplayWindow {
		std::shared_ptr<Gameboy> gb; 
		GLFWwindow* window;

		Texture tex;

		//maybe do something with these?
		u32 zoom = 3;
		bool grid = false;

	public:
		DisplayWindow(std::shared_ptr<Gameboy> gb, GLFWwindow* window) :
			gb(gb),
			window(window),
			tex(160, 144, gb->ppu.display.data())
		{}

		void render() {
			ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

			//Inputs
			if (ImGui::IsWindowFocused()) {
				handleInput(InputWindow::up, Button::Up);
				handleInput(InputWindow::down, Button::Down);
				handleInput(InputWindow::left, Button::Left);
				handleInput(InputWindow::right, Button::Right);
				handleInput(InputWindow::b, Button::B);
				handleInput(InputWindow::a, Button::A);
				handleInput(InputWindow::start, Button::Start);
				handleInput(InputWindow::select, Button::Select);
			}

			//Display
			std::unique_lock lock(gb->ppu.vblank_m);

			if (gb->ppu.presenting) {
				tex.mData = gb->ppu.displayPresent.data();
			}
			else {
				tex.mData = gb->ppu.display.data();
			}

			tex.update();
			tex.display(zoom, grid);

			/*
			gb->ppu.isVblank = true;
			gb->ppu.vblank.notify_one();
			*/

			ImGui::End();
		}

	private:
		//maybe add support for different types of "pressing" to activate?
		void handleInput(int keycode, Button button) {
			const int key = glfwGetKey(window, keycode);
			if (key == GLFW_PRESS) {
				gb->pad.pressButton(button);
			}
			else {
				gb->pad.releaseButton(button);
			}
		}
	};
}