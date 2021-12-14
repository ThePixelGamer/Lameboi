#include "Config.h"

#include <filesystem>
#include <fstream>

#include "PPU.h"
#include "util/StringUtils.h"

const std::string configPath = "lameboi.json";

namespace nlohmann {
template<>
struct adl_serializer<PaletteProfile> {
	static void to_json(json& j, const PaletteProfile& value) {
		for (auto& [name, palette] : value) {
			std::array<std::string, 4> paletteStr{};

			for (int i = 0; i < 4; ++i) {
				paletteStr[i] = '#' + hexToString(palette[i].getRGB());
			}

			j[name] = paletteStr;
		}
	}

	static void from_json(const json& j, PaletteProfile& value) {
		for (auto& [name, colArr] : j.items()) {
			Palette palette{};

			for (int i = 0; i < 4; ++i) {
				auto colStr = colArr[i].get<std::string>();

				// remove the # character
				colStr.erase(0, 1);

				palette[i] = stringToHex(colStr);
			}

			value.emplace(name, palette);
		}
	}
};
}

void Config::load() {
	using json = nlohmann::json;

	if (!std::filesystem::exists(configPath) || std::filesystem::is_empty(configPath)) {
		save();
	}

	std::ifstream cfg(configPath);
	json j;
	cfg >> j;

	// General/Input
	inputOverlay.deserialize(j, "inputOverlay");
	biosPath.deserialize(j, "biosPath");
	recentRoms.deserialize(j, "recentRoms");
	oppositeDir.deserialize(j, "oppositeDir");

	// Video
	paletteProfiles.deserialize(j, "palettes");

	if (j.contains("selectedPalette")) {
		currentPalette.deserialize(j["selectedPalette"]);
		PPU::paletteColors = paletteProfiles->at(currentPalette);
	}

	// Audio
	volume.deserialize(j, "volume");
	audioSync.deserialize(j, "audioSync");
}

void Config::save() {
	using json = nlohmann::json;

	std::ofstream cfg(configPath);
	json j;

	// General
	j["inputOverlay"] = inputOverlay.serialize();
	j["biosPath"] = biosPath.serialize();
	j["recentRoms"] = recentRoms.serialize();

	// Video
	j["selectedPalette"] = currentPalette.serialize();
	j["palettes"] = paletteProfiles.serialize();

	// Audio
	j["volume"] = volume.serialize();
	j["audioSync"] = audioSync.serialize();

	// Input
	j["oppositeDir"] = oppositeDir.serialize();

	cfg << std::setw(4) << j << std::endl;
}