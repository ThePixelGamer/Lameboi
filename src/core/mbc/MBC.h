#pragma once

#include "util/Types.h"

#include <algorithm>
#include <fstream>

class MBC {
protected:
	bool ram;
	bool battery;

public:
	virtual void setup(std::istream& stream) = 0;
	virtual void save() = 0;
	virtual void close() = 0;
	virtual void write(u16 location, u8 data) = 0;
	virtual u8 read(u16 location) = 0;

	static std::unique_ptr<MBC> createInstance(u8 type);

protected:
	MBC(bool ram_ = false, bool battery_ = false) :
		ram(ram_),
		battery(battery_) 
	{}

	std::string _getName(u8* start_) {
		std::string_view name(reinterpret_cast<const char*>(start_), 16);
		size_t trimPos = name.find('\0');
		if (trimPos != name.npos) {
			name.remove_suffix(name.size() - trimPos);
		}

		u8 end = *(start_ + 15);
		if ((name.size() == 16) && (end == 0x80 || end == 0xC0)) {
			name.remove_suffix(1);
		}

		return "saves/" + std::string(name) + ".lbs";
	}

	u16 _getMaxRomBanks(u8 type);
	u16 _getMaxRamBanks(u8 type);
};