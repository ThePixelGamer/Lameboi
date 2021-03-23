#pragma once

#include <memory>

#include "core/Gameboy.h"
#include "util/ImGuiHeaders.h"
#include "PFD.h"

namespace ui {
	class FileMenu {
		std::shared_ptr<Gameboy> gb;

		const std::vector<std::string> FileTypes{
			"Gameboy ROMs (.gb)", "*.gb",
			"Gameboy Compatible ROMs (.gbc)", "*.gbc",
			"All Files", "*"
		};

		const bool& debugger_opened;
		std::unique_ptr<pfd::open_file> open_file = nullptr;

	public:
		FileMenu(std::shared_ptr<Gameboy> gb, const bool& debug) :
			gb(gb),
			debugger_opened(debug) {}

		void render() {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)open_file);

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open Rom")) {
					gb->debug.continuing(false);
					gb->running = false;

					open_file = std::make_unique<pfd::open_file>("Choose file", "C:\\", FileTypes);
				}

				ImGui::EndMenu();
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

						gb->debug.continuing(!debugger_opened);

						std::thread emuthread(&Gameboy::Start, gb.get());
						emuthread.detach();
					}

					rom.close();
				}

				open_file = nullptr;
			}

			ImGui::PopItemFlag();
		}
	};
}