#include "Debugger.h"


void Debugger::addBreakpoint(u16 PC) {
	breakpoints.insert(PC);
}

Debugger::BreakpointIter Debugger::removeBreakpoint(BreakpointIter PC) {
	return breakpoints.erase(PC);
}

void Debugger::removeBreakpoint(u16 PC) {
	breakpoints.erase(PC);
}

std::set<u16>& Debugger::getBreakpoints() {
	return breakpoints;
}

void Debugger::continuing(bool cont) {
	canContinue = cont;
}

// to prevent the Show Debugger button from stopping the continue after hiding
bool Debugger::isContinuing() {
	return canContinue;
}

void Debugger::step(size_t steps_) {
	isStepping = true;
	steps = steps_;
}

size_t Debugger::amountToStep(u16 PC) {
	if (!breakpoints.empty()) {
		if (breakpoints.find(PC) != breakpoints.end()) {
			continuing(false);
			return 0;
		}
	}

	if (isStepping) {
		canContinue = false; //don't continue after stepping
		isStepping = false;
		return steps;
	}

	if (canContinue) {
		return 1;
	}

	return 0;
}