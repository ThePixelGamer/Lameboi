#include "FileMenu.h"

#include "util/SDLInterface.h"

const std::vector<std::string> gbFileTypes {
	"Gameboy ROMs (.gb)", "*.gb",
	"Gameboy Compatible ROMs (.gbc)", "*.gbc",
	"All Files", "*"
};

namespace ui {

void FileMenu::render() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)romFile);

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Open Rom")) {
			romFile = std::make_unique<pfd::open_file>("Select GB Rom", "C:\\", gbFileTypes);
		}

		if (ImGui::BeginMenu("Open Recent", config.recentRoms->size())) {
			auto& recentRoms = *config.recentRoms;
			for (size_t i = 0; i < config.maxRecentSize && i < recentRoms.size(); ++i) {
				auto& rom = recentRoms[i];
				if (!rom.empty() && ImGui::MenuItem(rom.c_str())) {
					openFile(rom);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) {
			loader.quit();
		}

		ImGui::EndMenu();
	}


	if (romFile && romFile->ready()) {
		auto result = romFile->result();
		if (!result.empty()) {
			openFile(result[0]);
		}

		romFile = nullptr;
	}

	ImGui::PopItemFlag();
}

void FileMenu::openFile(const std::string& filename) {
	// wait for the any emu threads to finish
	gb.stop();

	if (gb.loadRom(filename)) {
		fmt::print("Opened {}\n", filename);

		gb.debug.continuing(!debugger_opened);
		gb.start();
	}

	auto& recentRoms = *config.recentRoms;
	auto romIt = std::find(recentRoms.begin(), recentRoms.end(), filename);
	// bring file to the front
	if (romIt != recentRoms.end()) {
		std::rotate(recentRoms.begin(), romIt, std::next(romIt));
	}
	// push new file to the front
	else if (recentRoms.size() != config.maxRecentSize) {
		recentRoms.insert(recentRoms.begin(), filename);
	}
	// "remove" last element by replacing it with new file
	else {
		std::move_backward(recentRoms.begin(), std::prev(recentRoms.end()), recentRoms.end());
		recentRoms[0] = filename;
	}
}


} // namespace ui