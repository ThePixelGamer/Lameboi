#pragma once

#include <array>

#include "Util/Types.h"

class VRAMViewer {
public:
	std::array<u32, 256 * 256> bgmap0;
	std::array<u32, 128 * 64 * 3> tileData;

	VRAMViewer() {
		bgmap0.fill(0xFF); //alpha
		tileData.fill(0xFF);
	}
};