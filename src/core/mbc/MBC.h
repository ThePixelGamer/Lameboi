#pragma once

#include "util/Types.h"

#include <algorithm>
#include <fstream>

enum class CartridgeType : u8 {
	ROM_ONLY,
	MBC1 = 0x01,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2 = 0x05,
	MBC2_BATTERY,
	MMM01 = 0x0B, // not currently supported
	MMM01_RAM,
	MMM01_RAM_BATTERY,
	MBC3_TIMER_BATTERY = 0x0F,
	MBC3_TIMER_RAM_BATTERY,
	MBC3,
	MBC3_RAM,
	MBC3_RAM_BATTERY,
	MBC5 = 0x19,
	MBC5_RAM,
	MBC5_RAM_BATTERY,
	MBC5_RUMBLE,
	MBC5_RUMBLE_RAM,
	MBC5_RUMBLE_RAM_BATTERY,
};

class MBC {
protected:
	std::string romPath;
	bool ram;
	bool battery;

public:
	virtual void setup(std::istream& stream) = 0;
	virtual void save() = 0;
	virtual void close() = 0;
	virtual void write(u16 location, u8 data) = 0;
	virtual u8 read(u16 location) = 0;

	static std::unique_ptr<MBC> createInstance(const std::string& romPath_, u8 type);
	static const char* getTypeName(CartridgeType type);

protected:
	MBC(const std::string& romPath_, bool ram_ = false, bool battery_ = false) :
		romPath("saves/" + romPath + ".sav"),
		ram(ram_),
		battery(battery_) 
	{}

	std::string _getName(u8* start_) {
		/*
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
		*/

		return romPath;
	}

	u16 _getMaxRomBanks(u8 type);
	u16 _getMaxRamBanks(u8 type);
};