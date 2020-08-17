#pragma once

#include <array>

#include "Util/Types.h"

class VRAMViewer {
public:
	std::array<u32, 256 * 256> bgmap;
	std::array<u32, 128 * 64 * 3> tileData;
	std::array<u32, 64 * 40> oamData;

	VRAMViewer() {
		bgmap.fill(0xFF); //alpha
		tileData.fill(0xFF);
		oamData.fill(0xFF);
	}
};