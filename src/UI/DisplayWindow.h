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
			ImGui::Begin("Display");

			//Inputs
			//maybe add support for different types of "pressing" to activate?
			auto handleInput = [this](int keycode, Button button) {
				const int key = glfwGetKey(window, keycode);
				if (key == GLFW_PRESS) {
					gb->pad.pressButton(button);
				}
				else {
					gb->pad.releaseButton(button);
				}
			};

			if (ImGui::IsWindowFocused()) {
				handleInput(ui::InputWindow::up, Button::Up);
				handleInput(ui::InputWindow::down, Button::Down);
				handleInput(ui::InputWindow::left, Button::Left);
				handleInput(ui::InputWindow::right, Button::Right);
				handleInput(ui::InputWindow::b, Button::B);
				handleInput(ui::InputWindow::a, Button::A);
				handleInput(ui::InputWindow::start, Button::Start);
				handleInput(ui::InputWindow::select, Button::Select);
			}

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
	};
}