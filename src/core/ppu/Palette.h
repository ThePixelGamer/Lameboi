#pragma once

#include "util/types.h"

struct PaletteData {
	u8 color0 : 2;
	u8 color1 : 2;
	u8 color2 : 2;
	u8 color3 : 2;

	PaletteData() :
		color0(0),
		color1(0),
		color2(0),
		color3(0) 
	{}

	PaletteData(u8 c0, u8 c1, u8 c2, u8 c3) :
		color0(c0),
		color1(c1),
		color2(c2),
		color3(c3) 
	{}

	PaletteData& operator=(u8 c) {
		color0 = (c);
		color1 = (c >> 2);
		color2 = (c >> 4);
		color3 = (c >> 6);
		return *this;
	}

	bool operator==(PaletteData& pal) {
		return color0 == pal.color0
			&& color1 == pal.color1
			&& color2 == pal.color2
			&& color3 == pal.color3;
	}

	bool operator!=(PaletteData& pal) {
		return !(*this == pal);
	}

	u8 operator[](u8 idx) const {
		switch (idx) {
			case 0: return color0;
			case 1: return color1;
			case 2: return color2;
			case 3: return color3;
			default: return 0;
		}
	}

	u8 read() {
		return (color3 << 6) | (color2 << 4) | (color1 << 2) | (color0);
	}
};