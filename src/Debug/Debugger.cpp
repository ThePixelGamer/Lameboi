#include "Debugger.h"

#include "../Gameboy/Memory.h"

void Debugger::printSerialData() {
	if (mem.IORegs[0x02] == 0xFF) {
		printf("%c", mem.IORegs[0x01]);
		mem.Write(0xFF02, u8(0x00));
	}
}