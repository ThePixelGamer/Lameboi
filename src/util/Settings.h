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

	operator T() const {
		return value;
	}

	Setting& operator=(const T& v) {
		value = v;
		return *this;
	}

	json serialize() const { 
		return value;
	}

	void deserialize(const json& j) { 
		value = j.get<T>();
	}

	void reset() {
		value = defaultValue;
	}
};