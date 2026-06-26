#include "MainMenu.h"

#include "App.h"

namespace ui {

MainMenu::MainMenu(Gameboy& gb, App& context) :
	gb(gb),
	context(context),
	emuThread(&Gameboy::run, &gb) {

	// todo: call this somewhere else on startup?
	gb.loadBios(config.biosPath);
}

// todo: run on a separate thread to not be affected by UI performance?
void MainMenu::updateFPS() {
	perf = std::chrono::duration_cast<second>(clock::now() - perfTimer);
	if (perf.count() >= 1) {
		perfTimer = clock::now();
		fps = gb.ppu.framesPresented;
		gb.ppu.framesPresented = 0;
	}
}

const std::vector<std::string> gbFileTypes{
	"Gameboy ROMs (.gb)", "*.gb",
	"Gameboy Compatible ROMs (.gbc)", "*.gbc",
	"All Files", "*"
};

void MainMenu::renderFile() {
	auto openFile = [&](const std::string& filename) {
		// wait for the any emu threads to finish
		gb.stop();

		if (!std::filesystem::exists(filename)) {
			LB_INFO(Frontend, "File {} does not exist", filename);

			// Silently drop it from recent roms list if it exists
			auto& recentRoms = *config.recentRoms;
			auto romIt = std::find(recentRoms.begin(), recentRoms.end(), filename);
			if (romIt != recentRoms.end()) {
				recentRoms.erase(romIt);
			}

			return;
		}
		if (gb.loadRom(filename)) {
			fmt::print("Opened {}\n", filename);

			gb.debug.running = !context.debug.show;
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
			context.requestExit = true;
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

		if (ImGui::MenuItem("Show Display")) {
			context.display.show = true;
		}

		if (ImGui::MenuItem("Show Viewport")) {
			context.viewport.show = true;
		}

		if (ImGui::MenuItem("Settings")) {
			context.settings.show = true;
		}

		ImGui::EndMenu();
	}
}

void MainMenu::renderDebug() {
	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Show Debugger")) {
			context.debug.show = true;
			gb.debug.running = false;
		}

		if (ImGui::BeginMenu("PPU")) {
			ImGui::MenuItem("Background Map", nullptr, &context.bgmapWindow.show);
			ImGui::MenuItem("Tile Data", nullptr, &context.tileDataWindow.show);
			ImGui::MenuItem("OAM Sprites", nullptr, &context.oamWindow.show);

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

		ImGui::EndMainMenuBar();
	}

	// todo: move to it's own class or perhaps have the "status bar" be tied to the display window
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), window_flags)) {
		if (ImGui::BeginMenuBar()) {
			updateFPS();

			char v_str[64]{};
			ImFormatString(v_str, IM_ARRAYSIZE(v_str), "%lld FPS", fps);

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x - ImGui::CalcTextSize(v_str).x - ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text(v_str);
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
}

} // namespace ui