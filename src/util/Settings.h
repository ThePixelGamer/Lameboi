#pragma once

#include <string>
#include <utility>

#include "nlohmann/json.hpp"

template <typename T>
class Setting {
	using json = nlohmann::json;
	
	const T defaultValue;
	T value;

public:
	Setting(T val) : value(val), defaultValue(val) {}

	Setting& operator=(const T& v) {
		value = v;
		return *this;
	}

	operator T& () {
		return value;
	}

	T& get() {
		return value;
	}

	json serialize() const { 
		return value;
	}

	void deserialize(const json& j) { 
		value = j.get<T>();
	}

	void deserialize(const json& j, const char* label) {
		// switch this around to allow for exceptions and handle it in SettingsWindow.h?
		if (j.contains(label))
			deserialize(j[label]);
	}

	void reset() {
		value = defaultValue;
	}
};