#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "Types.h"

template <typename T>
bool inRange(const T& value, int low, int high) {
	return (value >= low) && (value <= high);
}

constexpr u8 getBit(u8 byte, u8 bit) {
	return (byte & (1 << bit)) != 0;
}

constexpr void setBit(u8& byte, u8 bit, u8 val) {
	byte &= ~(1 << bit);
	byte |= val << bit;
}

// todo: move
struct Pos2 {
	u32 x, y;

	bool operator==(const Pos2& rhs) const {
		return x == rhs.x && y == rhs.y;
	}

	bool operator!=(const Pos2& rhs) const {
		return !(*this == rhs);
	}

	bool operator<(const Pos2& rhs) const {
		return x < rhs.x && y < rhs.y;
	}

	friend void swap(Pos2& lhs, Pos2& rhs) {
		std::swap(lhs.x, rhs.x);
		std::swap(lhs.y, rhs.y);
	}
};