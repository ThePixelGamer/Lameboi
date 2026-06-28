#include "Debugger.h"

bool Debugger::shouldBreak(addr PC) {
	// have we already hit a breakpoint?
	if (!running) {
		return true;
	}

	// does the current instruction have a breakpoint?
	if (breakpoints.find(PC) != breakpoints.end()) {
		pause();
		return true;
	}

	// have we finished stepping?
	if (steps != 0 && --steps == 0) {
		pause();
		return true;
	}

	// are we in vblank?
	if (breakVblank && inVblank) {
		breakVblank = inVblank = false;

		pause();
		return true;
	}

	// no? continue :3
	return false;
}
