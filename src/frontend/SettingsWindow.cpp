#include "SettingsWindow.h"

#include <filesystem>

#include <imgui.h>
#include <fmt/format.h>

#include "core/Gameboy.h"
#include "core/Input.h"
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
	ImGui::InputText("##bios_path", config.biosPath.get()->data(), ImGuiInputTextFlags_ReadOnly);

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
	auto& c = PPU::paletteColors[paletteIdx];
	ImVec4 selCol = ImColor(c.r, c.g, c.b);
	if (ImGui::ColorEdit3("##ColorEditor", (float*)&selCol)) {
		PPU::paletteColors[paletteIdx] = (float*)&selCol;
	}

	for (u8 i = 0; i < 4; ++i) {
		std::string name = "##Color" + std::to_string(i);

		auto& c = PPU::paletteColors[i];
		ImVec4 col = ImColor(c.r, c.g, c.b);
		if (ImGui::ColorButton(name.c_str(), col))
			paletteIdx = i;

		if (i != 3)
			ImGui::SameLine();
	}

	if (ImGui::BeginCombo("##PaletteCombo", config.currentPalette->c_str())) {
		for (auto& [name, palette] : *config.paletteProfiles) {
			if (ImGui::Selectable(name.c_str())) {
				config.currentPalette = name;
				gb.ppu.setPalette(palette);
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
			// std::sort(palette.begin(), palette.end(), std::greater<Color>());

			config.currentPalette = std::filesystem::path(filePath).stem().string();
			(*config.paletteProfiles)[config.currentPalette] = palette;
			PPU::paletteColors = palette;
		}

		paletteFile = nullptr;
	}
}

void SettingsWindow::renderInputTab() {
	if (ImGui::BeginCombo("Input Keyboard", inputManager.keyboard.getName())) {
		for (auto keyboard : inputManager.keyboard.available) {
			std::string name = fmt::format("{}##{}", Input::Keyboard::getName(keyboard), keyboard);
			if (ImGui::Selectable(name.c_str())) {
				inputManager.keyboard.active = keyboard;
			}
		}

		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Input Gamepad", inputManager.gamepad.getName())) {
		for (auto gamepad : inputManager.gamepad.available) {
			std::string name = fmt::format("{}##{}", Input::Gamepad::getName(gamepad), gamepad);
			if (ImGui::Selectable(name.c_str())) {
				inputManager.gamepad.open(gamepad);
			}
		}

		ImGui::EndCombo();
	}

	ImVec2 start = ImGui::GetCursorScreenPos();
	float p = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) / ImGui::GetStyle().FontSizeBase;
	ImVec2 bsize = { p * 2, p * 2 };

	auto inputRemap = [this, &start, &bsize](Joypad::Button button, ImVec2 offset) {
		ImGui::SetCursorScreenPos(start + offset);
		const char* name = Joypad::names[button];
		bool activeGamepad = inputManager.gamepad.active != inputManager.gamepad.invalid;
		std::string namegui = fmt::format("{}##{}", inputManager.mapping.binds.at(name).getString(!activeGamepad), name);
		if (ImGui::Button(namegui.c_str(), bsize)) {
			if (inputManager.rebind == name) {
				inputManager.rebind = nullptr;
			}
			inputManager.rebind = name;
		}
	};

	ImColor dpadCol(39, 41, 41);
	ImColor bCol(154, 34, 87);
	ImColor sCol(114, 114, 114);
	ImColor textCol(72, 70, 134);
	ImGui::GetWindowDrawList()->AddRectFilled(start, start + ImGui::GetContentRegionAvail(), IM_COL32(196, 190, 187, 255));

	auto updateButtonColor = [](ImVec4 col) {
		ImGui::PushStyleColor(ImGuiCol_Button, col);

		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(h, s + .1f, v + .1f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(h, s + .2f, v + .2f));
	};

	ImGui::PushFont(nullptr, p * 0.75f);
	ImGui::GetWindowDrawList()->AddRectFilled(start + ImVec2(p*3, p*3), start + ImVec2(p*5, p*5), dpadCol);
	updateButtonColor(dpadCol);
	inputRemap(Joypad::Up, ImVec2{ p * 3, p });
	inputRemap(Joypad::Down, ImVec2{ p * 3, p * 5 });
	inputRemap(Joypad::Left, ImVec2{ p, p * 3 });
	inputRemap(Joypad::Right, ImVec2{ p * 5, p * 3 });
	ImGui::PopStyleColor(3);

	bsize = { p * 1.5f, p * 1.5f };
	updateButtonColor(bCol);

	//ImGui::GetWindowDrawList()->AddCircleFilled(start + ImVec2(p * 8.75, p * 5.75), p, bCol);
	//ImGui::GetWindowDrawList()->AddCircleFilled(start + ImVec2(p * 9.75, p * 2.75), p, bCol);
	inputRemap(Joypad::B, ImVec2{ p * 8, p * 5 }); ImGui::SameLine(); ImGui::TextColored(textCol, "B");
	inputRemap(Joypad::A, ImVec2{ p * 9, p * 2 }); ImGui::SameLine(); ImGui::TextColored(textCol, "A");
	ImGui::PopStyleColor(3);

	bsize = { p * 3, p * 1 };
	updateButtonColor(sCol);
	inputRemap(Joypad::Start, ImVec2{ p * 8, p * 8 }); ImGui::SameLine(); ImGui::TextColored(textCol, "Start");
	inputRemap(Joypad::Select, ImVec2{ p * 1, p * 8 }); ImGui::SameLine(); ImGui::TextColored(textCol, "Select");
	ImGui::PopStyleColor(3);
	ImGui::PopFont();
}

void SettingsWindow::renderGfxTab() {
	auto& spriteMgr = gb.spriteManager;
		
	auto& bios = spriteMgr.getManifest(true);
	if (!bios.skins.empty()) {
		if (ImGui::BeginCombo("Bios Skin", bios.selSkin.c_str())) {
			for (auto& skin : bios.skins) {
				if (ImGui::Selectable(skin.c_str())) {
					bios.selSkin = skin;
					gb.ppu.forceUpdate();
				}
			}

			ImGui::EndCombo();
		}
	}

	auto& game = spriteMgr.getManifest(false);
	if (!game.skins.empty()) {
		if (ImGui::BeginCombo("Game Skin", game.selSkin.c_str())) {
			for (auto& skin : game.skins) {
				if (ImGui::Selectable(skin.c_str())) {
					game.selSkin = skin;
					gb.ppu.forceUpdate();
				}
			}

			ImGui::EndCombo();
		}
	}
}

} // namespace ui
