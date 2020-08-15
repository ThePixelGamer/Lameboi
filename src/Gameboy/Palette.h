#pragma once

#include "Util/types.h"

struct Palette {
	u8 color0 : 2;
	u8 color1 : 2;
	u8 color2 : 2;
	u8 color3 : 2;

	bool operator==(Palette& pal) {
		return color0 == pal.color0
			&& color1 == pal.color1
			&& color2 == pal.color2
			&& color3 == pal.color3;
	}

	bool operator!=(Palette& pal) {
		return !(*this == pal);
	}
};