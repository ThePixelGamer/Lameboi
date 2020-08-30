#pragma once

#include <memory>

#include "mbc/IMBC.h"
#include "mbc/MBC0.h"
#include "mbc/MBC3.h"
#include "mbc/MBC5.h"

#include <fmt/printf.h>

inline std::unique_ptr<IMBC> loadMBCFromByte(u8 type) {
	//https://gbdev.io/pandocs/#_0147-cartridge-type
	switch (type) {
		case 0x00: return std::make_unique<MBC0>();
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
			fmt::printf("Unimplemented Cartridge Type 0x%02X\n", type);
			return nullptr;
	}
}