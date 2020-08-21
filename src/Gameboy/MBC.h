#pragma once

#include <memory>

#include "mbc/IMBC.h"
#include "mbc/MBC0.h"
#include "mbc/MBC3.h"

#include <fmt/printf.h>

inline std::unique_ptr<IMBC> loadMBCFromByte(u8 type) {
	switch (type) {
		case 0x0: return std::make_unique<MBC0>();
		case 0x10: return std::make_unique<MBC3>(true, true, true);
		case 0x11: return std::make_unique<MBC3>();
		case 0x12: return std::make_unique<MBC3>(true);
		case 0x13: return std::make_unique<MBC3>(true, true);

		default:
			fmt::printf("Unimplemented Cartridge Type 0x%02X\n", type);
			return nullptr;
	}
}