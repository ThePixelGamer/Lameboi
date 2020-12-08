#pragma once

#include "Util/Types.h"

class Interrupt {
	bool enableVblank;
	bool enableLcdStat;
	bool enableTimer;
	bool enableSerial;
	bool enableJoypad;

public:
	enum class Type : u8 {
		VBlank,
		LCD_STAT,
		Timer,
		Serial,
		Joypad
	};

	bool requestVblank;
	bool requestLcdStat;
	bool requestTimer;
	bool requestSerial;
	bool requestJoypad;

	void clean() {
		enableVblank = false;
		enableLcdStat = false;
		enableTimer = false;
		enableSerial = false;
		enableJoypad = false;

		requestVblank = false;
		requestLcdStat = false;
		requestTimer = false;
		requestSerial = false;
		requestJoypad = false;
	}

	bool shouldFire(Type type) {
		switch (type) {
			case Type::VBlank: return enableVblank && requestVblank;
			case Type::LCD_STAT: return enableLcdStat && requestLcdStat;
			case Type::Timer: return enableTimer && requestTimer;
			case Type::Serial: return enableSerial && requestSerial;
			case Type::Joypad: return enableJoypad && requestJoypad;
			
			default:
				//log
				return false;
		}
	}

	bool pending() {
		return requestVblank || requestLcdStat || requestTimer || requestSerial || requestJoypad;
	}

	u8 read(u8 reg) {
		if (reg == 0xFF) {
			return 0xE0 |
				(enableJoypad << 4) |
				(enableSerial << 3) |
				(enableTimer << 2) |
				(enableLcdStat << 1) |
				(enableVblank << 0);
		}
		else if (reg == 0x0F) {
			return 0xE0 |
				(requestJoypad << 4) |
				(requestSerial << 3) |
				(requestTimer << 2) |
				(requestLcdStat << 1) |
				(requestVblank << 0);
		}
		else {
			//log
			return 0xFF;
		}
	}

	void write(u8 reg, u8 value) {
		if (reg == 0xFF) {
			enableVblank = (value & 0x01);
			enableLcdStat = (value & 0x02);
			enableTimer = (value & 0x04); 
			enableSerial = (value & 0x08); 
			enableJoypad = (value & 0x10);
		}
		else if (reg == 0x0F) {
			requestVblank = (value & 0x01);
			requestLcdStat = (value & 0x02);
			requestTimer = (value & 0x04);
			requestSerial = (value & 0x08);
			requestJoypad = (value & 0x10);
		}
		else {
			//log
		}
	}
};