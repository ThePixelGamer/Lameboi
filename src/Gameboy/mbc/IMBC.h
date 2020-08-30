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

protected:
	std::string _getName(u8* start) {
		u8 end = *(start + 15);
		if (end == 0) {
			return reinterpret_cast<const char*>(start);
		}
		else if (end == 0x80 || end == 0xC0) {
			return std::string(reinterpret_cast<const char*>(start), 15);
		}
		else {
			return std::string(reinterpret_cast<const char*>(start), 16);
		}
	}

	//change this to use the size of the rom?
	u16 _getMaxRomBanks(u8 type) {
		switch (type) {
			case 0x1: return 4;
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