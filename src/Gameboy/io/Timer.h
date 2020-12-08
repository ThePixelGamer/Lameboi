#pragma once

#include "Util/Types.h"

class Interrupt;

class Timer {
	Interrupt& interrupt;

	//regs
	u8 DIV; //0xFF04 Divider Register
	u8 TIMA; //0xFF05 Timer Counter
	u8 TMA; //0xFF06 Timer Modulo
	struct {
		u8 clockSelect;
		bool timerOn;
	} TAC;

	//internal
	const u8 cyclesDivNeeds = 64; // amount of m-cycles to increment the div reg
	u8 divCycles;
	u16 currentCycleCount;

public:
	Timer(Interrupt& interrupt) : interrupt(interrupt) {
		clean();
	}

	void clean() {
		DIV = 0;
		TIMA = 0;
		TMA = 0;
		TAC.clockSelect = 0;
		TAC.timerOn = false;

		divCycles = 0;
		currentCycleCount = 0;
	}

	void update() {
		++divCycles;

		if ((divCycles % cyclesDivNeeds) == 0) {
			++DIV;
		}

		// todo implement the timer glitch mentioned in pandocs
		if (TAC.timerOn) {
			++currentCycleCount;

			constexpr std::array<u16, 4> timer{ 1024 / 4, 16 / 4, 64 / 4, 256 / 4 };

			if ((currentCycleCount % timer[TAC.clockSelect]) == 0) {
				if (++TIMA == 0) { // overflow
					TIMA = TMA;
					interrupt.requestTimer = true;
				}
			}

			if (currentCycleCount == timer[0]) { //1024 / 4
				currentCycleCount = 0;
			}
		}
		else {
			currentCycleCount = 0;
		}

		if (divCycles == cyclesDivNeeds) {
			divCycles = 0;
		}
	}

	u8 read(u8 reg) {
		switch (reg) {
			case 0x04: return DIV;
			case 0x05: return TIMA;
			case 0x06: return TMA;
			case 0x07: return 0xF8 | (TAC.timerOn << 2) | (TAC.clockSelect);

			default:
				//log
				return 0xFF;
		}
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
			case 0x04: 
				DIV = 0;
				divCycles = 0;
				break;

			case 0x05: TIMA = value; break;
			case 0x06: TMA = value; break;
			case 0x07:
				TAC.clockSelect = (value & 0x3);
				TAC.timerOn = (value & 0x4);
				break;

			default:
				//log
				break;
		}
	}
};