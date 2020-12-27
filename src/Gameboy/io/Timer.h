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
	const u8 cyclesDivNeeds = 256 / 4; // amount of m-cycles to increment the div reg
	u16 counter;

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

		counter = 0;
	}

	void update() {
		updateCounter(counter + 1);

		if ((counter & (cyclesDivNeeds - 1)) == 0) {
			++DIV;
		}

		if (counter == 0x100) { //1024 / 4
			counter = 0;
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
				updateCounter(0);
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

private:
	void updateCounter(u16 newVal) {
		constexpr std::array<u16, 4> timerMask { 1024 / 4 / 2, 16 / 4 / 2, 64 / 4 / 2, 256 / 4 / 2 };

		// todo implement the timer glitch
		if (TAC.timerOn) {
			u16 mask = timerMask[TAC.clockSelect];

			if ((counter & mask) == mask && (newVal & mask) != mask) {
				if (++TIMA == 0) { // overflow
					TIMA = TMA;
					interrupt.requestTimer = true;
				}
			}
		}

		counter = newVal;
	}
};