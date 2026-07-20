#pragma once

#include <array>
#include <functional>

#include "util/Types.h"

class Memory;
class Gameboy;
class Interrupt;

class CPU {
public:
	struct Flags {
		u8 nibble = 0;
		bool Z, N, H, C;

		operator u8() { return (Z << 7) | (N << 6) | (H << 5) | (C << 4); }
		void operator=(u8 val) {
			nibble = val & 0xF;

			Z = val & 0x80;
			N = val & 0x40;
			H = val & 0x20;
			C = val & 0x10;
		}
	};

	Flags F;
	u8 B, C, D, E, H, L, A;
	u8 Z, W;

	addr PC, SP;
	u8 IR;
	bool IME;

	bool lowPower;

	bool quit;

	using OP = void (CPU::*)();

private:
	using Handler = std::function<void (CPU*)>;
	using OpcodeTable = std::array<Handler, 256>;
	OpcodeTable opcodeTable, cbTable;
	Handler currentInstruction;

	std::function<void ()> stepComponents;
	Memory& bus;
	Interrupt& interrupt;

public:
	CPU(Gameboy& core);
	void update();
	void reset();

private:
	// Helper functions
	constexpr u8& reg(u8 r) {
		u8* regs[] = { &B, &C, &D, &E, &H, &L, &Z, &A };
		return *regs[r];
	}

	constexpr static u8 high(u16 hl) { return (hl >> 8); }
	constexpr static u8 low(u16 hl) { return hl & 0xFF; }
	constexpr static u16 to16(u8 h, u8 l) { return (h << 8) | l; }
	constexpr static void store16(u8& h, u8& l, u16 d) { h = high(d); l = low(d); }
	
	u8 read(addr a);
	u8 read(u8 a_h, u8 a_l) { return read(to16(a_h, a_l)); }
	u8 readHL() { return read(H, L); }
	u8 readN() { return read(PC++); }

	void loadZW() {
		Z = readN();
		W = readN();
	}
	
	void write(addr a, u8 data);
	void write(u8 a_h, u8 a_l, u8 data) { write(to16(a_h, a_l), data); }
	void writeHL(u8 data) { write(H, L, data); }

	void readHigh(u8 loc);
	void writeHigh(u8 loc);

	void invalid();
	void cb();
	void alu();

	// Instructions
	void nop() {}

	// 16-bit load
	void push(u8 h, u8 l) {
		write(--SP, h);
		write(--SP, l);
	}

	void pop(u8& h, u8& l) {
		l = read(SP++);
		h = read(SP++);
	}

	void loadSP() {	
		loadZW();
		SP = (W << 8) | Z; 
	}

	// 8-bit load
	void load() {
		u8& dst = reg((IR >> 3) & 0x7);
		u8& src = reg(IR & 0x7);

		if (&src == &Z && (IR & 0x40)) Z = readHL();
		dst = src;
		if (&dst == &Z) writeHL(Z);
	}

	// rotate/shift/bit instructions
	void rotateLeft(u8& reg, bool circular = false) {
		bool c = (reg & 0x80);
		reg <<= 1;
		reg |= u8((circular) ? c : F.C);

		F.Z = reg == 0;
		F.N = false;
		F.H = false;
		F.C = c;
	}

	void rotateRight(u8& reg, bool circular = false) {
		bool c = (reg & 0x1);
		reg >>= 1;
		reg |= ((circular) ? c : F.C) << 7;

		F.Z = reg == 0;
		F.N = false;
		F.H = false;
		F.C = c;
	}

	void shiftLA(u8& reg) {
		F.C = reg & 0x80;
		reg <<= 1;
		F.Z = reg == 0;
		F.H = false;
		F.N = false;
	}
	
	void shiftRA(u8& reg) {
		F.C = reg & 0x1;
		reg = (reg & 0x80) | (reg >> 1);
		F.Z = reg == 0;
		F.H = false;
		F.N = false;
	}

	void swap(u8& reg) {
		reg = (reg << 4) | (reg >> 4);

		F.Z = reg == 0;
		F.C = false;
		F.H = false;
		F.N = false;
	}

	void shiftRL(u8& reg) {
		F.C = reg & 0x1;
		reg >>= 1;
		F.Z = reg == 0;
		F.H = false;
		F.N = false;
	}

	void bit(u8 reg, u8 bit) {
		F.Z = (reg & (1 << bit)) == 0;
		F.N = false;
		F.H = true;
	}

	void reset(u8& reg, u8 bit) {
		reg &= ~(1 << bit);
	}

	void set(u8& reg, u8 bit) {
		reg |= (1 << bit);
	}

	// 16-bit alu
	void add16(u8 h, u8 l) {
		_add(L, l);
		stepComponents();
		_add(H, h, F.C);
	}

	void inc16(u8& h, u8& l) {
		u16 res = to16(h, l) + 1;
		store16(h, l, res);
	}

	void dec16(u8& h, u8& l) {
		u16 res = to16(h, l) - 1;
		store16(h, l, res);
	}

	// 8-bit alu
	void daa() {
		if (!F.N) {
			if (F.C || A > 0x99) {
				A += 0x60;
				F.C = true;
			}

			if (F.H || (A & 0xF) > 0x9) {
				A += 0x6;
			}
		}
		else {
			if (F.C) {
				A -= 0x60;
			}

			if (F.H) {
				A -= 0x6;
			}
		}

		F.Z = A == 0;
		F.H = false;
	}

	void cpl() {
		A = ~A;
		F.N = true;
		F.H = true;
	}

	void scf() {
		F.C = true;
		F.N = false;
		F.H = false;
	}

	void ccf() {
		F.C = !F.C;
		F.N = false;
		F.H = false;
	}

	void _add(u8& r, u8 val, bool carry = false) {
		u16 res = r + val + carry;
		F.H = (r ^ val ^ res) & 0x10; 
		F.C = res > 0xFF;
		r = res;
		F.N = false;
	}

	void _sub(u8& r, u8 val, bool carry = false) {
		s16 res = r - val - carry;
		F.H = (r ^ val ^ res) & 0x10;
		F.C = res < 0;
		r = res;
		F.N = true;
	}

	void inc() {
		u8& r = reg((IR >> 3) & 0x7);
		bool isZ = &r == &Z;
		if (isZ) Z = readHL();

		F.H = ((r & 0xf) + 1) > 0xf;
		++r;
		F.Z = r == 0;
		F.N = false;

		if (isZ) writeHL(Z);
	}

	void dec() { 
		u8& r = reg((IR >> 3) & 0x7);
		bool isZ = &r == &Z;
		if (isZ) Z = readHL();

		F.H = ((r & 0xf) - 1) < 0;
		--r;
		F.Z = r == 0;
		F.N = true;

		if (isZ) writeHL(Z);
	}

	void _and() {
		A &= reg(IR & 0x7);
		F.Z = A == 0;
		F.N = false;
		F.H = true;
		F.C = false;
	}

	void _xor() {
		A ^= reg(IR & 0x7);
		F.Z = A == 0;
		F.N = false;
		F.H = false;
		F.C = false;
	}

	void _or() {
		A |= reg(IR & 0x7);
		F.Z = A == 0;
		F.N = false;
		F.H = false;
		F.C = false;
	}

	void _cp() {
		u8 tmp = reg(IR & 0x7);

		F.Z = A == tmp;
		F.N = true;
		F.H = (A & 0xF) < (tmp & 0xF);
		F.C = A < tmp;
	}

	// control flow instructions
	bool cond() {
		// 0x20/0xC0 = F.Z, 0x30/0xD0 = F.C
		auto& flag = (IR & 0x10) ? F.C : F.Z;
		// 0x8 = true, 0x0 = false
		return flag == bool(IR & 0x8);
	}

	void _call(addr newPC) {
		// cycle to account for --SP;
		stepComponents();
		push(high(PC), low(PC));
		PC = newPC;
	}

	void _jmp(u8 h, u8 l) {
		PC = to16(h, l);
	}

	void rst() {
		_call(((IR >> 3) & 0x7) * 8);
	}

	void relJump() {
		Z = readN();

		// cc
		if (IR & 0x20) {
			if (!cond()) return;
		}

		PC += s8(Z);
		stepComponents();
	}

	void jmp() {
		loadZW();

		if (!(IR & 0x1)) {
			if (!cond()) return;
		}

		stepComponents();
		_jmp(W, Z);
	}

	void ret() {
		if (!(IR & 0x1)) {
			if (!cond()) return;
		}

		pop(W, Z);
		stepComponents();
		_jmp(W, Z);
	}

	void call() {
		loadZW();
		
		if (IR != 0xCD) {
			if (!cond()) return;
		}

		_call(to16(W, Z));
	}
};
