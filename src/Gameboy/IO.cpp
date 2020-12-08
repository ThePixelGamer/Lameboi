#include "IO.h"

#include "Gameboy.h"

bool IO::unused(u8 reg) {
	//non-ranges
	bool extra = (reg == 0x03) || (reg == 0x15) || (reg == 0x1F);

	//not cgb
	if (true) { 
		extra = extra || (reg == 0x4F) || inRange(reg, 0x51, 0x55) || inRange(reg, 0x68, 0x70);
	}

	return extra || inRange(reg, 0x08, 0x0E) || inRange(reg, 0x27, 0x2F) || 
		inRange(reg, 0x4C, 0x4E) || inRange(reg, 0x56, 0x67) || inRange(reg, 0x71, 0x7F);
}

u8 IO::read(u8 reg) {
	if (unused(reg)) {
		return 0xFF;
	}

	if (reg == 0x00) {
		return gb.joypad.read();
	}
	else if (inRange(reg, 0x01, 0x02)) {
		return gb.serial.read(reg);
	}
	else if (inRange(reg, 0x04, 0x07)) {
		return gb.timer.read(reg);
	}
	else if (inRange(reg, 0x10, 0x3F)) {
		return gb.apu.read(reg);
	}
	else if (inRange(reg, 0x40, 0x4B)) {
		return gb.ppu.read(reg);
	}
	else if (reg == 0x50) {
		return gb.mem.boot;
	}

	//log
	return 0xFF;
}

void IO::write(u8 reg, u8 value) {
	if (unused(reg)) {
		return;
	}
	
	if (reg == 0x00) {
		gb.joypad.write(value);
	}
	else if (inRange(reg, 0x01, 0x02)) {
		gb.serial.write(reg, value);
	}
	else if (inRange(reg, 0x04, 0x07)) {
		gb.timer.write(reg, value);
	}
	else if (inRange(reg, 0x10, 0x3F)) {
		gb.apu.write(reg, value);
	}
	else if (inRange(reg, 0x40, 0x4B)) {
		gb.ppu.write(reg, value);
	}
	else if (reg == 0x50) {
		gb.mem.boot = !value;
	}
	else {
		//log
	}
}