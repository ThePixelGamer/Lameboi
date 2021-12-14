#include "MBC.h"

#include "MBC0.h"
#include "MBC1.h"
#include "MBC3.h"
#include "MBC5.h"

#include <fmt/printf.h>

//https://gbdev.io/pandocs/#_0147-cartridge-type
std::unique_ptr<MBC> MBC::createInstance(const std::string& romPath, u8 type) {
	switch (type) {
		case 0x00: return std::make_unique<MBC0>(romPath);
		case 0x01: return std::make_unique<MBC1>(romPath);
		case 0x02: return std::make_unique<MBC1>(romPath, true);
		case 0x03: return std::make_unique<MBC1>(romPath, true, true);
		case 0x10: return std::make_unique<MBC3>(romPath, true, true, true);
		case 0x11: return std::make_unique<MBC3>(romPath);
		case 0x12: return std::make_unique<MBC3>(romPath, true);
		case 0x13: return std::make_unique<MBC3>(romPath, true, true);
		case 0x19: return std::make_unique<MBC5>(romPath);
		case 0x1A: return std::make_unique<MBC5>(romPath, true);
		case 0x1B: return std::make_unique<MBC5>(romPath, true, true);
		case 0x1C: return std::make_unique<MBC5>(romPath, false, false, true);
		case 0x1D: return std::make_unique<MBC5>(romPath, true, false, true);
		case 0x1E: return std::make_unique<MBC5>(romPath, true, true, true);

		default:
			fmt::printf("Unimplemented Cartridge Type: 0x%02X\n", type);
			return nullptr;
	}
}

const char* MBC::getTypeName(CartridgeType type) {
	switch (type) {
		case CartridgeType::ROM_ONLY:
			return "ROM ONLY";

		case CartridgeType::MBC1:
			return "MBC1";
		case CartridgeType::MBC1_RAM:
			return "MBC1+RAM";
		case CartridgeType::MBC1_RAM_BATTERY:
			return "MBC1+RAM+BATTERY";

		case CartridgeType::MBC2:
			return "MBC2";
		case CartridgeType::MBC2_BATTERY:
			return "MBC2+BATTERY";

		case CartridgeType::MBC3_TIMER_BATTERY:
			return "MBC3+TIMER+BATTERY";
		case CartridgeType::MBC3_TIMER_RAM_BATTERY:
			return "MBC3+TIMER+RAM+BATTERY";
		case CartridgeType::MBC3:
			return "MBC3";
		case CartridgeType::MBC3_RAM:
			return "MBC3+RAM";
		case CartridgeType::MBC3_RAM_BATTERY:
			return "MBC3+RAM+BATTERY";

		case CartridgeType::MBC5:
			return "MBC5";
		case CartridgeType::MBC5_RAM:
			return "MBC5+RAM";
		case CartridgeType::MBC5_RAM_BATTERY:
			return "MBC5+RAM+BATTERY";
		case CartridgeType::MBC5_RUMBLE:
			return "MBC5+RUMBLE";
		case CartridgeType::MBC5_RUMBLE_RAM:
			return "MBC5+RUMBLE+RAM";
		case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
			return "MBC5+RUMBLE+RAM+BATTERY";

		default:
			return "Unknown Cartridge Type";
	}
}

//change this to use the size of the rom?
//https://gbdev.io/pandocs/#_0148-rom-size
u16 MBC::_getMaxRomBanks(u8 type) {
	switch (type) {
		case 0x0: return 2;
		case 0x1: return 4;
		case 0x2: return 8;
		case 0x3: return 16;
		case 0x4: return 32;
		case 0x5: return 64;
		case 0x6: return 128;
		case 0x7: return 128; //a test does this, not sure if any commerical roms does it
		default:
			fmt::printf("Unhandled Rom Bank Type: 0x%01X\n", type);
			return 0;
	}
}

//https://gbdev.io/pandocs/#_0149-ram-size
u16 MBC::_getMaxRamBanks(u8 type) {
	switch (type) {
		case 0x1: return 1; //not sure what to do in this case
		case 0x2: return 1;
		case 0x3: return 4;
		case 0x4: return 16;
		case 0x5: return 8;
		default:
			fmt::printf("Unhandled Ram Bank Type: 0x%01X\n", type);
			return 0;
	}
}