#pragma once

#include "util/Types.h"

class SerialPort {
	u8 data;
	bool useInternalClock;
	bool requestTransfer;

public:
	SerialPort() {
		clean();
	}

	void clean() {
		data = 0;
		useInternalClock = true;
		requestTransfer = false;
	}

	u8 read(u8 reg) {
		switch (reg) {
			case 0x01: return data;
			case 0x02: return (requestTransfer << 7) | 0x7E | (useInternalClock << 0);

			default:
				//log
				return 0xFF;
		}
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
			case 0x01: data = value; break;
			case 0x02:
				useInternalClock = (value & 0x1);
				requestTransfer = (value & 0x80);
				break;

			default:
				//log
				break;
		}
	}

	void print() {
		if (requestTransfer) {
			printf("%c", data);
			requestTransfer = false;
		}
	}
};