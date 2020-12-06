#pragma once

#include "Util/Types.h"

#include <algorithm>
#include <fstream>

class IMBC {
protected:
	bool ram;
	bool battery;

public:
	virtual void setup(std::istream& stream) = 0;
	virtual void save() = 0;
	virtual void close() = 0;
	virtual void write(u16 location, u8 data) = 0;
	virtual u8 read(u16 location) = 0;

protected:
	IMBC(bool ram_ = false, bool battery_ = false) :
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

	//change this to use the size of the rom?
	u16 _getMaxRomBanks(u8 type) {
		switch (type) {
			case 0x1: return 4;
			case 0x4: return 32;
			case 0x5: return 64;
			case 0x6: return 128;
			case 0x7: return 128; //a test does this, not sure if any commerical roms does it
			default:
				std::cout << "Unhandled Rom Bank Type {" << std::hex << +type << "}" << std::endl;
				return 0;
		}
	}

	u16 _getMaxRamBanks(u8 type) {
		switch (type) {
			case 0x1: return 1; //not sure what to do in this case
			case 0x2: return 1;
			case 0x3: return 4;
			case 0x4: return 16;
			case 0x5: return 8;
			default:
				std::cout << "Unhandled Ram Bank Type {" << std::hex << +type << "}" << std::endl;
				return 0;
		}
	}
};