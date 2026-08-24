#include "Config.h"

#include <filesystem>
#include <fstream>

#include "PPU.h"
#include "util/StringUtils.h"

const std::string configPath = "lameboi.json";

namespace Model {

NLOHMANN_JSON_SERIALIZE_ENUM(Type, {
	{AUTO, "auto"},
	{DMG, "dmg"},
	{CGB, "cgb"},
})
	
}

namespace nlohmann {
template<>
struct adl_serializer<PaletteProfile> {
	static void to_json(json& j, const PaletteProfile& value) {
		for (auto& [name, palette] : value) {
			std::array<std::string, 4> paletteStr{};

			for (int i = 0; i < 4; ++i) {
				paletteStr[i] = '#' + hexToString(palette[i].r << 16 | palette[i].g << 8 | palette[i].b);
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

	// General
	inputOverlay.deserialize(j, "inputOverlay");
	biosDir.deserialize(j, "biosDir");
	fastBios.deserialize(j, "fastBios");
	recentRoms.deserialize(j, "recentRoms");
	model.deserialize(j, "model");
	
	// Video
	paletteProfiles.deserialize(j, "palettes");

	if (j.contains("selectedPalette")) {
		currentPalette.deserialize(j["selectedPalette"]);
		PPU::paletteColors = paletteProfiles->at(currentPalette);
	}
	
	// Audio
	volume.deserialize(j, "volume");
	audioSync.deserialize(j, "audioSync");
	
	// Input
	oppositeDir.deserialize(j, "oppositeDir");
}

void Config::save() {
	using json = nlohmann::json;

	std::ofstream cfg(configPath);
	json j;

	// General
	j["inputOverlay"] = inputOverlay.serialize();
	j["biosPath"] = biosDir.serialize();
	j["fastBios"] = fastBios.serialize();
	j["recentRoms"] = recentRoms.serialize();
	j["model"] = model.serialize();

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
