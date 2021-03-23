#pragma once

#include "util/Types.h"

class Interrupt;

class Timer {
	Interrupt& interrupt;

	//regs
	u16 counter; //MSB: 0xFF04 Divider Register
	u8 TIMA; //0xFF05 Timer Counter
	u8 TMA; //0xFF06 Timer Modulo
	struct {
		u8 clockSelect;
		bool timerOn;
	} TAC;

	
	//internal
	constexpr static std::array<u16, 4> timerMask{ 1024 / 2, 16 / 2, 64 / 2, 256 / 2 };

	bool timerOverflow;
	bool reloaded;

public:
	Timer(Interrupt& interrupt) : interrupt(interrupt) {
		clean();
	}

	void clean() {
		counter = 0;
		TIMA = 0;
		TMA = 0;
		TAC.clockSelect = 0;
		TAC.timerOn = false;

		timerOverflow = false;
		reloaded = false;
	}

	void update() {
		u16 oldCounter = counter;
		counter += 4;

		reloaded = false;

		if (timerOverflow) {
			timerOverflow = false;

			TIMA = TMA;
			reloaded = true;
			interrupt.requestTimer = true;
		}
		else if (TAC.timerOn) {
			u16 mask = timerMask[TAC.clockSelect];

			if ((oldCounter & mask) && (counter & mask) == 0) {
				increaseTIMA();
			}
		}
	}

	u8 read(u8 reg) {
		switch (reg) {
			case 0x04: return counter >> 8;
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
			case 0x04: // DIV
				if (counter & timerMask[TAC.clockSelect]) {
					increaseTIMA();
				}

				counter = 0;
				break;

			case 0x05: // TIMA
				if (!reloaded) {
					timerOverflow = false;
					TIMA = value;
				}
				break;

			case 0x06: // TMA
				if (reloaded) {
					TIMA = value;
				}
				
				TMA = value; 
				break;

			case 0x07: { // TAC
				u8 clock = (value & 0x3);
				bool on = (value & 0x4);
				
				bool oldBit = TAC.timerOn && (counter & timerMask[TAC.clockSelect]);
				bool newBit = on && (counter & timerMask[clock]);
				if (oldBit && !newBit) {
					increaseTIMA();
				}

				TAC.clockSelect = clock;
				TAC.timerOn = on;
			} break;

			default:
				//log
				break;
		}
	}

private:
	void increaseTIMA() {
		if (++TIMA == 0) { // overflow
			timerOverflow = true;
		}
	}
};