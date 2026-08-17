#pragma once

#include "Model.h"
#include "util/MemoryMap.h"
#include "util/Types.h"

#include <array>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <cstddef>

class Cartridge;
class Memory;

// Abstract/Bare metal implementation
class MBC {
protected:
	Cartridge& hw;

public:
	MBC(Cartridge& hw) : hw(hw) {}
	virtual ~MBC() = default;
	virtual void install() = 0;
};

constexpr std::size_t ROM_BANK_SIZE = 0x4000;
constexpr std::size_t RAM_BANK_SIZE = 0x2000;

class Cartridge {
public:
	enum Hardware {
		BARE,
		MBC1,
		MBC2,
		MMM01,
		MBC3,
		MBC5,
		MBC6,
		MBC7,
		Camera,
		BANDAI_TAMA5,
		HuC3,
		HuC1,
		_MAX,
		HW_MASK = 0xF,

		RAM,
		BATTERY = RAM << 1,
		TIMER = BATTERY << 1,
		RUMBLE = TIMER << 1,
		PART_MASK = RAM | BATTERY | TIMER | RUMBLE,
	}; 
	static_assert(_MAX < RAM);
	
	friend Hardware operator|(Hardware l, Hardware r) { return Hardware((int)l | (int)r); }
	static std::string getHWName(Hardware hw);

	// https://gbdev.io/pandocs/The_Cartridge_Header.html
	struct alignas(1) Header {
		static constexpr u16 BASE = 0x100;

		u8 entryPoint[0x4];
		u8 nLogo[0x30];

		union {
			u8 v1Title[16];
			u8 v2Title[15];

			struct {
				u8 title[11];
				u8 manuCode[4];
				u8 cgbFlag;
			};

			u8 raw[0x10];
		};

		u8 nLCode[0x2];
		bool sgbFlag;

		enum Type : u8 {
			ROM_ONLY = 0x00,
			ROM_RAM = 0x08,
			ROM_RAM_BATTERY = 0x09,

			MBC1_ROM = 0x01,
			MBC1_RAM = 0x02,
			MBC1_RAM_BATTERY = 0x03,

			MBC2_ROM = 0x05,
			MBC2_BATTERY = 0x06,

			MMM01_ROM = 0x0B,
			MMM01_RAM = 0x0C,
			MMM01_RAM_BATTERY = 0x0D,

			MBC3_TIMER_BATTERY = 0x0F,
			MBC3_TIMER_RAM_BATTERY = 0x10,
			MBC3_ROM = 0x11,
			MBC3_RAM = 0x12,
			MBC3_RAM_BATTERY = 0x13,

			MBC5_ROM = 0x19,
			MBC5_RAM = 0x1A,
			MBC5_RAM_BATTERY = 0x1B,
			MBC5_RUMBLE = 0x1C,
			MBC5_RUMBLE_RAM = 0x1D,
			MBC5_RUMBLE_RAM_BATTERY = 0x1E,

			MBC6 = 0x20,
			MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
			POCKET_CAMERA = 0xFC,
			BANDAI_TAMA5 = 0xFD,
			HUC3 = 0xFE,
			HUC1_RAM_BATTERY = 0xFF
		} cartType;

		u8 romSizeCode;
		u8 ramSizeCode;

		u8 destCode;
		u8 oLCode;
		u8 romVersion;
		u8 headerChecksum;
		u8 highChecksum, lowChecksum;

		Header() = delete;

		constexpr std::size_t getMaxRomBanks() {
			// todo: verify but it seems to me that the 2 nibbles are used to support bank amounts outside the power of 2
			// ex: $10 = 6 banks or in pandocs $52 = 72 banks
			// upperNibble listed as only being 5 in pandocs
			const u8 upperNibble = (romSizeCode >> 4) & 0xF; 
			const u8 lowerNibble = romSizeCode & 0xF;

			return bool(upperNibble) * (2 << upperNibble) + (2 << lowerNibble);
		}

		constexpr std::size_t getMaxRamBanks() {
			constexpr u8 ramBank[] = { 0, 0, 1, 4, 16, 8 };
			assert(ramSizeCode < sizeof(ramBank));
			return ramBank[ramSizeCode];
		}
		
		Model::Type getModel() {
			if (cgbFlag & 0x80) {
				return Model::CGB;
			}
			else {
				return Model::DMG;
			}
		}
	};
	static_assert(sizeof(Header) == (0x150 - Header::BASE), "Alignas failed");

	static_assert(offsetof(Header, cartType) == (0x147 - Header::BASE));
	static_assert(offsetof(Header, romSizeCode) == (0x148 - Header::BASE));
	static_assert(offsetof(Header, ramSizeCode) == (0x149 - Header::BASE));

	Hardware components = BARE;

	MemoryMap rom_backing;
	MemoryMap::Section rom;
	std::size_t romSize = 0;
	
	MemoryMap ram_backing;
	MemoryMap::Section ram;
	std::size_t ramSize = 0;
	
	u8 bank0, bankRam;
	u16 bank1;
	MemoryMap::ReservedSection rom0, rom1;
	MemoryMap::ReservedSection ram0_tag, ram1_tag;
	MemoryMap::ReservedSection ram0;

	Memory& bus;
	std::unique_ptr<class MBC> mbc;
	
	bool connected = false;
	std::string romName;

	Cartridge(Memory& bus);

	~Cartridge() {
		unload();
	}

	bool load(const std::filesystem::path& romPath);
	void unload();

	// helper functions
	operator bool() {
		return rom.get();
	}

	bool has(const Hardware part) {
		return ((components & PART_MASK) & part) == part;
	}

	bool is(const Hardware type) {
		return (components & HW_MASK) == type;
	}

	Header* getHeader() {
		return reinterpret_cast<Header*>(rom.get() + Header::BASE);
	}

	void switchBank0(u8 bank);
	void switchBank1(u16 bank);
	void enableRam(bool enable);
	void switchRam(u8 bank);

private:
	const std::filesystem::path getSavePath() {
		return "saves/" + romName + ".sav";
	}

	void _initHW();
};
