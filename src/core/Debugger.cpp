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

void Debugger::step(size_t steps_) {
	isStepping = true;
	steps = steps_;
}

size_t Debugger::amountToStep(u16 PC) {
	if (!breakpoints.empty()) {
		if (breakpoints.find(PC) != breakpoints.end()) {
			running = false;
			return 0;
		}
	}

	if (isStepping) {
		running = false; //don't continue after stepping
		isStepping = false;
		return steps;
	}

	if (running) {
		return 1;
	}

	return 0;
}