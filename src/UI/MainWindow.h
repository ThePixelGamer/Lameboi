#pragma once

#include <memory>

#include "Gameboy/Gameboy.h"
#include "Util/ImGuiHeaders.h"
#include "PFD.h"

namespace ui {
	class MainWindow {
		const std::vector<std::string> FileTypes {
			"Gameboy ROMs (.gb)", "*.gb",
			"Gameboy Compatible ROMs (.gbc)", "*.gbc",
			"All Files", "*"
		};

		ImGuiIO& io = ImGui::GetIO(); 
		std::unique_ptr<pfd::open_file> open_file = nullptr;

		std::shared_ptr<Gameboy> gb;

		bool& show_debug_window;
		bool& show_ppu_window;
		bool& show_inputs_window;

	public:
		MainWindow(std::shared_ptr<Gameboy> gb, bool& debug, bool& ppu, bool& inputs) :
			gb(gb),
			show_debug_window(debug),
			show_ppu_window(ppu),
			show_inputs_window(inputs)
		{}

		void render() {
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Lameboi");

			ImGui::Text("UI average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)open_file);

			if (ImGui::Button("Open File")) {
				gb->debug.continuing(false);
				gb->running = false;

				open_file = std::make_unique<pfd::open_file>("Choose file", "C:\\", FileTypes);
			}

			// this only works because the file dialog takes a while to choose one :)
			if (open_file && open_file->ready()) {
				//wait for the any emu threads to finish
				{
					std::unique_lock lock(gb->emustart_m);
					gb->emustart.wait(lock, [this] { return gb->finished; });
				}

				auto result = open_file->result();
				if (!result.empty()) {
					std::string& file = result[0];

					std::ifstream rom(file, std::ifstream::binary);

					if (gb->LoadRom(rom)) {
						std::cout << "Opened file " << file << "\n";

						if (!show_debug_window) {
							gb->debug.continuing(true);
						}

						std::thread emuthread(&Gameboy::Start, gb.get());
						emuthread.detach();
					}

					rom.close();
				}

				open_file = nullptr;
			}

			ImGui::PopItemFlag();


			if (ImGui::Button("Continue")) {
				gb->debug.continuing(true);
			}

			ImGui::SameLine();
			if (ImGui::Button("Pause")) {
				gb->debug.continuing(false);
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				gb->debug.continuing(false);
				gb->running = false;
			}

			ImGui::End();


			ImGui::Begin("Gameboy");

			if (ImGui::Button("Show PPU")) {
				show_ppu_window = !show_ppu_window;
			}

			if (ImGui::Button("Show Debugger")) {
				show_debug_window = !show_debug_window;
				gb->debug.continuing((show_debug_window) ? false : gb->debug.isContinuing());
			}

			ImGui::End();
		}
	};
}