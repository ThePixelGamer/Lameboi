#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS //imgui_mem_editor

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "Gameboy/Gameboy.h"

#include "UI/DisplayWindow.h"
#include "UI/DebugWindow.h"
#include "UI/PPUWindow.h"
#include "UI/MainWindow.h"

#include "imgui_mem_editor.h"

int main(int, char**) {
	GLFWImGui context;

	if (context.error) {
		return 1;
	}

	ImGuiIO& io = ImGui::GetIO();

	bool show_demo_window = true;
	bool show_emu_window = true;
	bool show_debug_window = false;
	bool show_cpu_window = false;
	bool show_mem_window = false;

	static MemoryEditor mem_edit;

	auto gb = std::make_shared<Gameboy>();
	
	ui::DisplayWindow display(gb);
	ui::DebugWindow debug(gb); //todo make these bools a window class
	ui::PPUWindow ppu(gb); //might have to make this a unique_ptr to avoid filling stack
	bool inputs = false;
	ui::MainWindow menu(gb, debug.show, ppu.show, inputs);

	if (!std::filesystem::exists("saves")) {
		std::filesystem::create_directories("saves");
	}

	// Main loop
	while(context.NewFrame()) {
		menu.render();
		display.render();

		ppu.render();
		debug.render();

		//Memory
		//mem_edit.DrawWindow("Rom Bank 0", &mem.mbc->romBank0, 0x4000);
		//mem_edit.DrawWindow("Rom Bank N", mem.mbc->current, 0x4000, 0x4000);
		//mem_edit.DrawWindow("Memory", &gb->mem, sizeof(gb->mem) - sizeof(&gb->mem.gb), 0x8000);
		
		//Input, not hiding this behind an if statement because it would block inputs
		{
			ImGui::Begin("Inputs");

			//maybe add support for different types of "pressing" to activate?
			auto handleInput = [&gb, &window = context.window](int keycode, Button button) -> bool {
				const int key = glfwGetKey(window, keycode);
				if (key == GLFW_PRESS) {
					gb->pad.pressButton(button);
					return true;
				}
				else {
					gb->pad.releaseButton(button);
					return false;
				}
			};

			if (handleInput(GLFW_KEY_W, Button::Up)) {
				ImGui::Text("Up");
			}

			if (handleInput(GLFW_KEY_S, Button::Down)) {
				ImGui::Text("Down");
			}

			if (handleInput(GLFW_KEY_A, Button::Left)) {
				ImGui::Text("Left");
			}

			if (handleInput(GLFW_KEY_D, Button::Right)) {
				ImGui::Text("Right");
			}

			if (handleInput(GLFW_KEY_K, Button::B)) {
				ImGui::Text("B");
			}

			if (handleInput(GLFW_KEY_L, Button::A)) {
				ImGui::Text("A");
			}

			if (handleInput(GLFW_KEY_ENTER, Button::Start)) {
				ImGui::Text("Start");
			}

			if (handleInput(GLFW_KEY_E, Button::Select)) {
				ImGui::Text("Select");
			}

			ImGui::End();
		}

		context.Render();
	}

	if (gb->running && gb->mbc) {
		gb->running = false;
		gb->mbc->close();
	}
	
	return 0;
}
