#pragma once

#include <array>
#include "util/Types.h"

struct Color {
	// for compliance with imgui color editor
	float r, g, b, a;

	Color() : Color(0xffffffff) {}

	Color(u8 _r, u8 _g, u8 _b, u8 _a = 0xff) {
		setRGB(_r, _g, _b, _a);
	}

	// todo: add some & 0xFF, implicit *should* take care of it
	Color(u32 rgba) : Color(rgba >> 24, rgba >> 16, rgba >> 8, rgba) {}

	Color& operator=(u32 rgba) {
		setRGB(rgba >> 24, rgba >> 16, rgba >> 8, rgba);
		return *this;
	}

	void setRGB(u8 _r, u8 _g, u8 _b, u8 _a = 0xff) {
		r = _r / 255.0f;
		g = _g / 255.0f;
		b = _b / 255.0f;
		a = _a / 255.0f;
	}

	operator u32() const {
		u32 out = 0;
		out = static_cast<u8>(r * 255.0f) << 24;
		out |= static_cast<u8>(g * 255.0f) << 16;
		out |= static_cast<u8>(b * 255.0f) << 8;
		out |= static_cast<u8>(a * 255.0f);
		return out;
	}

	// helper function for imgui
	float* data() {
		return &r;
	}
};

using Palette = std::array<Color, 4>;