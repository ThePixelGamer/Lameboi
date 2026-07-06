#pragma once

#include <array>

#include "util/Types.h"

// Color3/RGB implementation
struct Color {
	u8 r, g, b, a;
	
	Color() : Color(0xFF, 0xFF, 0xFF, 0xFF) {}
	Color(u8 r, u8 g, u8 b, u8 a = 0xFF) : r(r), g(g), b(b), a(a) {}
	Color(float c[3]) : Color(u8(c[0] * 255.f), u8(c[1] * 255.f), u8(c[2] * 255.f), 0xFF) {}
	Color(u32 rgb) : Color(rgb >> 16, rgb >> 8, rgb, 0xFF) {}

	bool operator==(const Color& c) const {
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}
};

using Palette = std::array<Color, 4>;