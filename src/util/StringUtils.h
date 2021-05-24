#pragma once

#include <charconv>
#include <string>
#include <sstream>

#include "Types.h"

inline u32 stringToHex(const std::string& str) {
	u32 result{};

	auto [p, ec] = std::from_chars(str.c_str(), str.c_str() + str.size(), result, 16);
	if (ec != std::errc()) {
		// log?
	}

	return result;
}

inline std::string hexToString(u32 value) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(6);
	ss << std::hex << value;
	return ss.str();
}