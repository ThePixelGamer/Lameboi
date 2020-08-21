#pragma once

#include "Util/Types.h"

#include <fstream>

class IMBC {
public:
	bool ram = false;
	bool battery = false;

	virtual void setup(std::istream& stream) = 0;
	virtual void save() = 0;
	virtual void close() = 0;
	virtual void write(u16 location, u8 data) = 0;
	virtual u8 read(u16 location) = 0;
};