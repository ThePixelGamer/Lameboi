#pragma once

#include "Types.h"

template <typename H, typename L>
class RegPair {
public:
	H high;
	L low;

	operator u16() const {
		return (high << 8) | low;
	}

	void operator=(u16 val) {
		high = val >> 8;
		low = val & 0xFF;
	}

	RegPair& operator&=(u16 val) {
		*this = *this & val;
		return *this;
	}

	RegPair operator++(int) {
		RegPair old = *this;
		*this = old + 1;
		return old;
	}

	RegPair operator--(int) {
		RegPair old = *this;
		*this = old - 1;
		return old;
	}
};