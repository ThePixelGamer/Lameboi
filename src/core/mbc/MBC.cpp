#include "MBC.h"

#include "MBC0.h"
#include "MBC1.h"
#include "MBC3.h"
#include "MBC5.h"

#include <fmt/printf.h>

//https://gbdev.io/pandocs/#_0147-cartridge-type
std::unique_ptr<MBC> MBC::createInstance(u8 type) {
	switch (type) {
		case 0x00: return std::make_unique<MBC0>();
		case 0x01: return std::make_unique<MBC1>();
		case 0x02: return std::make_unique<MBC1>(true);
		case 0x03: return std::make_unique<MBC1>(true, true);
		case 0x10: return std::make_unique<MBC3>(true, true, true);
		case 0x11: return std::make_unique<MBC3>();
		case 0x12: return std::make_unique<MBC3>(true);
		case 0x13: return std::make_unique<MBC3>(true, true);
		case 0x19: return std::make_unique<MBC5>();
		case 0x1A: return std::make_unique<MBC5>(true);
		case 0x1B: return std::make_unique<MBC5>(true, true);
		case 0x1C: return std::make_unique<MBC5>(false, false, true);
		case 0x1D: return std::make_unique<MBC5>(true, false, true);
		case 0x1E: return std::make_unique<MBC5>(true, true, true);

		default:
			fmt::printf("Unimplemented Cartridge Type: 0x%02X\n", type);
			return nullptr;
	}
}

//change this to use the size of the rom?
//https://gbdev.io/pandocs/#_0148-rom-size
u16 MBC::_getMaxRomBanks(u8 type) {
	switch (type) {
		case 0x0: return 2;
		case 0x1: return 4;
		case 0x2: return 8;
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