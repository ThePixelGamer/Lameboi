#pragma once

#include <string>
#include <filesystem>

inline void createDirectory(std::string folder) {
	if (!std::filesystem::exists(folder)) {
		std::filesystem::create_directories(folder);
	}
}