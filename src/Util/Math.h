#pragma once

namespace math {
	constexpr double pi = 3.14159265358979323846;

	template<typename T>
	T sign(T x) {
		if (x >= 0) {
			return static_cast<T>(1);
		}
		else {
			return static_cast<T>(-1);
		}
	}
}