#include "Scheduler.h"

#include "Gameboy.h"

void Scheduler::newMCycle() {
	gb.mem.update();
	gb.ppu.update(); 
	
	++currentCycleCount;

	if ((currentCycleCount % cyclesDiv) == 0) {
		++gb.mem.DIV;
	}

	// need to implement the timer glitch mentioned in pandocs
	if (gb.mem.TAC.timerOn) {
		constexpr std::array<u16, 4> timer { 1024, 16, 64, 256 };

		if ((currentCycleCount % timer[gb.mem.TAC.clockSelect]) == 0) {
			if (++gb.mem.TIMA == 0) { // overflow
				gb.mem.TIMA = gb.mem.TMA;
				gb.mem.IF.timer = 1;
			}
		}
	}
 
	if (currentCycleCount == 1024) {
		currentCycleCount = 0;
	}
}