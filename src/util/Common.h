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