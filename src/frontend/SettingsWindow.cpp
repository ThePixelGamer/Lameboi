#include "SettingsWindow.h"

#include <filesystem>

#include "core/Gameboy.h"
#include "core/Input.h"
#include "util/ImGuiHeaders.h"
#include "util/StringUtils.h"

const std::vector<std::string> palletteFileTypes{
	"Lospec Palette (.hex)", "*.hex",
	"All Files", "*"
};

const std::vector<std::string> biosFileTypes{
	"Boot Rom Files", "*.bin",
	"All Files", "*"
};

namespace ui {

void SettingsWindow::render() {
	if (show) {
		ImGui::Begin("Settings", &show);

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)paletteFile || (bool)biosFile);

		if (ImGui::Button("Save")) {
			config.save();
		}

		if (ImGui::BeginTabBar("##SettingsTabBar", ImGuiTabBarFlags_NoTooltip)) {
			if (ImGui::BeginTabItem("General")) {
				renderGeneralTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Video")) {
				renderVideoTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Audio")) {
				ImGui::SliderInt("Volume", config.volume.get(), 0, 100);
				ImGui::Checkbox("Sync to Audio", config.audioSync.get());
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Input")) {
				renderInputTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("CGfx")) {
				renderGfxTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		/* no worky, also would this actually be better than it being at the top?
		ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetItemRectSize().y);

		if (ImGui::Button("Save")) {
			saveCfg();
		}
		*/

		ImGui::PopItemFlag();

		ImGui::End();
	}
}

void SettingsWindow::renderGeneralTab() {
	ImGui::InputText("##bios_path", config.biosPath->data(), config.biosPath->size(), ImGuiInputTextFlags_ReadOnly);

	ImGui::SameLine();
	if (ImGui::Button("Select")) {
		biosFile = std::make_unique<pfd::open_file>("Select bios file", "C:\\", biosFileTypes, true);
	}

	if (biosFile && biosFile->ready()) {
		auto result = biosFile->result();
		if (!result.empty()) {
			std::string& file = result[0];

			gb.loadBios(file);
			config.biosPath = file;
		}

		biosFile = nullptr;
	}

	ImGui::Checkbox("Enable input overlay", config.inputOverlay.get());
	ImGui::Checkbox("Allow L+R or U+D inputs", config.oppositeDir.get());
}

void SettingsWindow::renderVideoTab() {
	ImGui::ColorEdit3("##ColorEditor", PPU::paletteColors[paletteIdx].data());

	for (u8 i = 0; i < 4; ++i) {
		std::string name = "##Color" + std::to_string(i);
		Color& color = PPU::paletteColors[i];
		if (ImGui::ColorButton(name.c_str(), ImVec4(color.r, color.g, color.b, 1.0f)))
			paletteIdx = i;

		if (i != 3)
			ImGui::SameLine();
	}

	if (ImGui::BeginCombo("##PaletteCombo", config.currentPalette->c_str())) {
		for (auto& [name, palette] : *config.paletteProfiles) {
			if (ImGui::Selectable(name.c_str())) {
				config.currentPalette = name;
				PPU::paletteColors = palette;
			}
		}

		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("Import")) {
		paletteFile = std::make_unique<pfd::open_file>("Select palette file", "C:\\", palletteFileTypes, true);
	}

	if (paletteFile && paletteFile->ready()) {
		for (auto& filePath : paletteFile->result()) {
			std::ifstream file(filePath);

			Palette palette{};
			std::string line;
			for (int i = 0; (i < 4) && std::getline(file, line); ++i) {
				// lospec .hex files are stored from dark -> light usually
				palette[3 - i] = stringToHex(line);
			}

			// ensure light -> dark palette
			std::sort(palette.begin(), palette.end(), std::greater<u32>());

			config.currentPalette = std::filesystem::path(filePath).stem().string();
			(*config.paletteProfiles)[config.currentPalette] = palette;
			PPU::paletteColors = palette;
		}

		paletteFile = nullptr;
	}
}

void SettingsWindow::renderInputTab() {
	const char* deviceName = (inputManager.gamepadActive) ? inputManager.getControllerName(inputManager.selectedPad) : "Keyboard";
	if (ImGui::BeginCombo("Input Device##controllers", deviceName)) {
		if (ImGui::Selectable("Keyboard")) {
			inputManager.gamepadActive = false;
		}

		for (auto [id, pad] : inputManager.getControllers()) {
			if (ImGui::Selectable(pad.name)) {
				inputManager.gamepadActive = true;
				inputManager.selectedPad = id;
			}
		}
		ImGui::EndCombo();
	}

	auto inputRemap = [this](GB::Button button) {
		constexpr const char* names[] = {
			"Up", "Down", "Left", "Right",
			"Start", "Select", "B", "A"
		};

		const char* name = names[button];
		std::string namegui = inputManager.getButtonName(name) + "##" + name;
		if (ImGui::Button(namegui.c_str())) {
			remapButton = button;
		}

		if (remapButton == button && inputManager.remapButton(name)) {
			remapButton = GB::NumButtons;
		}

		ImGui::SameLine(); ImGui::Text(name);
	};

	inputRemap(GB::Up);
	inputRemap(GB::Down);
	inputRemap(GB::Left);
	inputRemap(GB::Right);
	inputRemap(GB::B);
	inputRemap(GB::A);
	inputRemap(GB::Start);
	inputRemap(GB::Select);
}

void manifestCombo(SpriteManager& spriteMgr, bool inBios) {
	auto& manifest = spriteMgr.getManifest(inBios);
	const char* combo = (inBios) ? "Bios Profile" : "Game Profile";

	if (manifest.profiles.empty())
		return;

	auto& currentProfile = manifest.getCurrentProfile();
	if (ImGui::BeginCombo(combo, currentProfile.name.c_str())) {
		for (int i = 0; i < manifest.profiles.size(); ++i) {
			auto& profile = manifest.profiles[i];

			if (ImGui::Selectable(profile.name.c_str())) {
				manifest.currentProfile = i;
			}
		}

		ImGui::EndCombo();
	}

	combo = (inBios) ? "Bios Skin" : "Game Skin";

	if (currentProfile.loadedSkins.empty())
		return;

	auto& skinName = currentProfile.getCurrentSkin().name;
	if (skinName.empty())
		return;
	
	if (ImGui::BeginCombo(combo, skinName.c_str())) {
		for (int i = 0; i < currentProfile.loadedSkins.size(); ++i) {
			auto& skin = currentProfile.loadedSkins[i];

			if (ImGui::Selectable(skin.name.c_str())) {
				currentProfile.currentSkin = i;
			}
		}

		ImGui::EndCombo();
	}
}

void SettingsWindow::renderGfxTab() {
	auto& spriteMgr = gb.spriteManager;

	manifestCombo(spriteMgr, true);
	manifestCombo(spriteMgr, false);
}

} // namespace ui