#pragma once

namespace Model {

// unsure if I want to emulate all of these options
enum Type {
	AUTO,
	DMG,
	SGB,
	MGB,
	CGB,
	AGB,
	GBP
};

inline const char* to_str(Model::Type type) {
	switch (type) {
		case AUTO: return "Auto";
		case DMG: return "DMG";
		case CGB: return "CGB";
		default: return "Unsupported";
	}
}

}
