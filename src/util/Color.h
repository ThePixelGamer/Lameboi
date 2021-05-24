#pragma once

#include <array>

#include "util/Types.h"

// Color3/RGB implementation
struct Color {
	float r, g, b;

	Color() : Color(0xff, 0xff, 0xff) {}

	Color(u8 _r, u8 _g, u8 _b) {
		setRGB(_r, _g, _b);
	}

	// todo: add some & 0xFF, implicit *should* take care of it
	Color(u32 rgb) : Color(rgb >> 16, rgb >> 8, rgb) {}

	Color& operator=(u32 rgb) {
		setRGB(rgb >> 16, rgb >> 8, rgb);
		return *this;
	}

	// returns RGBA for opengl textures
	operator u32() const {
		return (getRGB() << 8) | 0xFF;
	}

	void setRGB(u8 _r, u8 _g, u8 _b) {
		r = _r / 255.0f;
		g = _g / 255.0f;
		b = _b / 255.0f;
	}

	u32 getRGB() const {
		u32 out = 0;
		out = static_cast<u8>(r * 255.0f) << 16;
		out |= static_cast<u8>(g * 255.0f) << 8;
		out |= static_cast<u8>(b * 255.0f);
		return out;
	}

	// helper function for imgui
	float* data() {
		return &r;
	}
};

using Palette = std::array<Color, 4>;