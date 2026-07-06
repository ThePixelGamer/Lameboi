#include "MainMenu.h"

#include "core/Gameboy.h"
#include "MainWindow.h"

namespace ui {

MainMenu::MainMenu(UI& app) : gb(app.gb), app(app) {}

const std::vector<std::string> gbFileTypes{
	"Gameboy ROMs (.gb)", "*.gb",
	"Gameboy Compatible ROMs (.gbc)", "*.gbc",
	"All Files", "*"
};

void MainMenu::renderFile() {
	auto openFile = [&](const std::string& filename) {
		// wait for the any emu threads to finish
		gb.stop();

		if (!gb.loadRom(filename, !app.debug.show)) {
			LB_INFO(Frontend, "File {} does not exist", filename);

			// Silently drop it from recent roms list if it exists
			auto& recentRoms = *config.recentRoms;
			auto romIt = std::find(recentRoms.begin(), recentRoms.end(), filename);
			if (romIt != recentRoms.end()) {
				recentRoms.erase(romIt);
			}

			return;
		}

		LB_INFO(Frontend, "Opened {}", filename);

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
	};

	//ImGui::BeginDisabled();

	if (ImGui::BeginMenu("File", !romFile)) {
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
			//app.requestExit = true;
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

	//ImGui::EndDisabled();
}

void MainMenu::renderGameboy() {
	if (ImGui::BeginMenu("Gameboy")) {
		if (ImGui::MenuItem("Pause", nullptr, !gb.debug.running)) {
			gb.debug.running = !gb.debug.running;
		}

		if (ImGui::MenuItem("Show Viewport", nullptr, app.viewport.show)) {
			app.viewport.show = true;
		}

		if (ImGui::MenuItem("Settings", nullptr, app.settings.show)) {
			app.settings.show = true;
		}

		ImGui::EndMenu();
	}
}

void MainMenu::renderDebug() {
	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Show Debugger", nullptr, app.debug.show)) {
			app.debug.show = true;
			gb.debug.pause();
		}

		if (ImGui::BeginMenu("PPU")) {
			ImGui::MenuItem("Background Map", nullptr, &app.bgmapWindow.show);
			ImGui::MenuItem("Tile Data", nullptr, &app.tileDataWindow.show);
			ImGui::MenuItem("OAM Sprites", nullptr, &app.oamWindow.show);

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::render() {
	if (ImGui::BeginMainMenuBar()) {
		renderFile();
		renderGameboy();
		renderDebug();

		if (ImGui::BeginMenu("Tools")) {
			ImGui::MenuItem("Show ImGui DemoWindow", nullptr, &showDemoWindow);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	ImGui::ShowDemoWindow(&showDemoWindow);
}

} // namespace ui