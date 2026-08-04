#pragma once

#include <cstdio>

#include "util/Types.h"

#include "../Memory.h"

class SerialPort {
private:
	u8 data;
	bool useInternalClock;
	bool requestTransfer;

public:
	SerialPort(Memory& bus) {
		clean();

		auto serial_tag = bus.register_bus(
			[](void* d, addr a) -> u8 { 
				return static_cast<SerialPort*>(d)->read(a & 0xFF);
			},
			[](void* d, addr a, u8 v) { 
				static_cast<SerialPort*>(d)->write(a & 0xFF, v);
			},
			this
		);

		bus.register_io(0x1, serial_tag);
		bus.register_io(0x2, serial_tag);
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
