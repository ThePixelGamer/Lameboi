#pragma once

#include "IMBC.h"

class MBC3 : public IMBC {
	bool timer = false; 
	bool ramEnabled = false;
	u8 romBank = 1; // 1h-7Fh
	u8 ramBank = 0; // 0h-3h = banks, 
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
	MBC3(bool ram_ = false, bool battery_ = false, bool timer_ = false) : IMBC(),
		timer(timer_) {
		battery = battery_;
		ram = ram_;

		romBanks.fill({});
		ramBanks.fill({});
	}

	std::string _getName() {
		if (romBanks[0][0x143] == 0) {
			return reinterpret_cast<const char*>(&romBanks[0][0x134]);
		}
		else if (romBanks[0][0x143] == 0x80 || romBanks[0][0x143] == 0xC0) {
			return std::string(reinterpret_cast<const char*>(&romBanks[0][0x134]), 11);
		}
		else {
			return std::string(reinterpret_cast<const char*>(&romBanks[0][0x134]), 15);
		}
	}

	//change this to use the size of the rom?
	u16 _getMaxBanks(u8 type) {
		switch (type) {
			case 0x1: return 4;
			case 0x5: return 64;
			case 0x6: return 128;
			case 0x7: return 128; //a test does this, not sure if any commerical roms does it
			default: 
				std::cout << "Unhandled Bank Type {" << std::hex << +type << "}" << std::endl; 
				return 0;
		}
	}

	virtual void setup(std::istream& stream) {
		stream.read((char*)romBanks[0].data(), 0x4000);

		u16 maxBanks = _getMaxBanks(romBanks[0][0x148]);

		// 01h - Amount of Banks
		for (u8 bankAmount = 1; bankAmount < maxBanks; ++bankAmount) {
			stream.read((char*)romBanks[bankAmount].data(), 0x4000);
		}

		//probably change this to only save the ram banks used
		if (battery) {
			std::ifstream file("saves/" + _getName() + ".lbs", std::ios_base::binary);

			for (size_t i = 0; i < ramBanks.size(); ++i) {
				file.read((char*)ramBanks[i].data(), 0x2000);
			}

			file.close();
		}
	}

	virtual void save() {
		// dump ram to file
		if (battery) {
			std::ofstream file("saves/" + _getName() + ".lbs", std::ios_base::binary);

			for (size_t i = 0; i < ramBanks.size(); ++i) {
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
		if (location >= 0xA000) {
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
			else if (ramEnabled) {
				ramBanks[ramBank][location - 0xA000] = data;
			}
		}
		else if (location >= 0x6000) {
			// update the RTC registers?
		}
		else if (location >= 0x4000) {
			if (timer && (data & 0x8)) {
				ramOrRtc = true;
				rtcRegister = (data & 0xF) - 8;
			}
			else if (ram) {
				ramOrRtc = false;
				ramBank = (data & 0x3);
			}
		}
		else if (location >= 0x2000) {
			romBank = (data & 0x7F);
			if (romBank == 0) {
				romBank = 1;
			}
		}
		else {
			if ((data & 0xF) == 0xA) {
				ramEnabled = true;
			}
			else {
				ramEnabled = false;
			}
		}
	}

	virtual u8 read(u16 location) {
		if (location >= 0xA000) {
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
			else if (ramEnabled) {
				return ramBanks[ramBank][location - 0xA000];
			}
		}
		else if (location >= 0x4000) {
			return romBanks[romBank][location - 0x4000];
		}
		else {
			return romBanks[0][location];
		}

		return 0xFF;
	}
};