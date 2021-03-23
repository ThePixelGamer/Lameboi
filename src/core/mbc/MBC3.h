#pragma once

#include "MBC.h"
#include "util/Common.h"

#include <fmt/format.h>

class MBC3 : public MBC {
	bool timer = false; 
	bool ramEnabled = false;
	u8 romBank = 1; // 1h-7Fh
	u8 ramBank = 0; // 0h-3h = banks
	u8 rtcRegister = 0; // 0h-4h = RTC registers
	bool ramOrRtc = false; // false = ram, true = rtc

	std::array<std::array<u8, 0x4000>, 0x80> romBanks; // 00h-7Fh
	std::array<std::array<u8, 0x2000>, 0x4> ramBanks; // 00h-03h

	// unimplemented, use std::chrono?
	u8 rtcS = 0;
	u8 rtcM = 0;
	u8 rtcH = 0;
	union {
		struct {
			u16 counter : 9;
			u8 : 5;
			u8 halt : 1;
			u8 carry : 1;
		};

		u16 rawValue = 0x3E00;
	} rtcD;

public:
	MBC3(bool ram_ = false, bool battery_ = false, bool timer_ = false) : MBC(ram_, battery_), timer(timer_) {
		romBanks.fill({});
		ramBanks.fill({});
	}

	virtual void setup(std::istream& stream) {
		stream.read((char*)romBanks[0].data(), 0x4000);

		u16 maxBanks = _getMaxRomBanks(romBanks[0][0x148]);

		// 01h - Amount of Banks
		for (u8 bankAmount = 1; bankAmount < maxBanks; ++bankAmount) {
			stream.read((char*)romBanks[bankAmount].data(), 0x4000);
		}

		//probably change this to only save the ram banks used
		if (battery) {
			std::ifstream file(_getName(&romBanks[0][0x134]), std::ios_base::binary);
			maxBanks = _getMaxRamBanks(romBanks[0][0x149]);

			for (size_t i = 0; i < maxBanks; ++i) {
				file.read((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void save() {
		// dump ram to file
		if (battery) {
			std::ofstream file(_getName(&romBanks[0][0x134]), std::ios_base::binary);
			u16 maxBanks = _getMaxRamBanks(romBanks[0][0x149]);

			for (size_t i = 0; i < maxBanks; ++i) {
				file.write((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void close() {
		save();

		for (auto& bank : romBanks) {
			bank.fill(0xFF);
		}

		for (auto& bank : ramBanks) {
			bank.fill(0xFF);
		}
	}

	virtual void write(u16 location, u8 data) {
		u8 type = location >> 12;

		switch (type) {
			case 0x0: case 0x1:
				ramEnabled = (data & 0xF) == 0xA;
				break;

			case 0x2: case 0x3:
				romBank = (data & 0x7F);
				if (romBank == 0) {
					romBank = 1;
				}
				break;

			case 0x4: case 0x5:
				if (timer && (data & 0x8)) {
					ramOrRtc = true;
					rtcRegister = (data & 0xF) - 8;
				}
				else if (ram) {
					ramOrRtc = false;
					ramBank = (data & 0x3);
				}
				break;

			case 0x6: case 0x7:
				// update the RTC registers?
				break;

			case 0xA: case 0xB:
				if (ramOrRtc) {
					switch (rtcRegister) {
						case 0x0: rtcS = data; break;
						case 0x1: rtcM = data; break;
						case 0x2: rtcH = data; break;
						case 0x3: rtcD.counter = data; break;
						case 0x4:
							rtcD.counter = (data << 8) | (rtcD.counter & 0xFF);
							rtcD.halt = (data >> 6);
							rtcD.carry = (data >> 7);
							break;
						default: break;
					}
				}
				else if (ramEnabled && ram) {
					ramBanks[ramBank][location & 0x1FFF] = data;
				}
				break;

			default:
				fmt::print("ur dumb {:x}", location);
				break;
		}
	}

	virtual u8 read(u16 location) {
		u8 type = location >> 12;

		switch (type) {
			case 0x0: case 0x1: case 0x2: case 0x3:
				return romBanks[0][location];

			case 0x4: case 0x5: case 0x6: case 0x7:
				return romBanks[romBank][location & 0x3FFF];

			case 0xA: case 0xB:
				if (ramOrRtc && timer) {
					switch (rtcRegister) {
						case 0x0: return rtcS;
						case 0x1: return rtcM;
						case 0x2: return rtcH;
						case 0x3: return (rtcD.rawValue & 0xFF);
						case 0x4: return (rtcD.rawValue >> 8);
						default: break;
					}
				}
				else if (ramEnabled && ram) {
					return ramBanks[ramBank][location & 0x1FFF];
				}
				break;

			default:
				fmt::print("ur dumb {:x}", location);
				break;
		}

		return 0xFF;
	}
};