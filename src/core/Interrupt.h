#pragma once

#include "util/Types.h"

class Interrupt {
public:
	enum Type : u8 {
		VBlank,
		LCD_STAT,
		Timer,
		Serial,
		Joypad
	};

	struct Flags {
		bool vblank = false;
		bool lcdStat = false;
		bool timer = false;
		bool serial = false;
		bool joypad = false;

		bool operator[](Type idx) {
			switch (idx) {
				case VBlank: return vblank;
				case LCD_STAT: return lcdStat;
				case Timer: return timer;
				case Serial: return serial;
				case Joypad: return joypad;
				
				default: return false;
			}
		}

		void clean() {
			vblank = false;
			lcdStat = false;
			timer = false;
			serial = false;
			joypad = false;
		}

		void write(u8 value) {
			vblank = (value & 0x01);
			lcdStat = (value & 0x02);
			timer = (value & 0x04);
			serial = (value & 0x08);
			joypad = (value & 0x10);
		}

		u8 read() {
			return 0xE0 |
				(joypad << 4) |
				(serial << 3) |
				(timer << 2) |
				(lcdStat << 1) |
				(vblank << 0);
		}
	} enable, request;

	Interrupt() {
		clean();
	}

	void clean() {
		enable.clean();
		request.clean();
	}

	bool shouldFire(Type type) {
		return enable[type] && request[type];
	}

	bool pending() {
		return request.read() != 0xE0;
	}
};