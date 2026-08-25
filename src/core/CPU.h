#pragma once

#include <array>
#include <atomic>
#include <functional>

#include <fmt/format.h>

#include "Model.h"
#include "util/Types.h"

class Memory;
class Gameboy;
class Interrupt;
class SpriteManager;

class CPU {
public:
	using ReferenceData = std::pair<u8, u32>;

	struct Flags {
		u8 nibble = 0;
		bool Z, N, H;
		
		struct Carry : public std::pair<bool, u32> {
			Carry& operator=(const std::pair<bool, u32>& rhs) {
				std::pair<bool, u32>::operator=(rhs);
				return *this;
			}

			Carry& operator=(bool v) {
				first = v;
				second = 0;
				return *this;
			}
			
			operator bool&() {
				return first;
			}
		} C;

		operator u8() { return (Z << 7) | (N << 6) | (H << 5) | (C << 4); }
		void operator=(u8 val) {
			nibble = val & 0xF;

			Z = val & 0x80;
			N = val & 0x40;
			H = val & 0x20;
			C = val & 0x10;
		}
	};

	struct Register : public ReferenceData {
		Register& operator=(const Register& rhs) = default;
		Register& operator=(const ReferenceData& rhs) {
			ReferenceData::operator=(rhs);
			return *this;
		}

		Register& operator=(u8 v) {
			first = v;
			second = 0;
			return *this;
		}
		
		operator u8&() {
			return first;
		}
	};

	Flags F;
	Register B, C, D, E, H, L, A;
	Register Z, W;

	addr PC, SP;
	u8 IR;
	bool IME;

	bool haltBug;
	bool handler;
	bool lowPower;

	bool quit;
	std::atomic<uint64_t> instrCount;

	std::vector<u8> bios;
	bool inBios;

	bool doubleSpeed;
	bool speedSwitch;

private:
	Gameboy& core;
	Memory& bus;
	Interrupt& interrupt;

public:
	CPU(Gameboy& core);
	void install(Memory& bus, Model::Type model);
	void update();
	void reset();

	std::string log();

private:
	// Helper functions
	void stepComponents();

	constexpr Register& reg(u8 r) {
		Register* regs[] = { &B, &C, &D, &E, &H, &L, &Z, &A };
		return *regs[r];
	}

	constexpr static u8 high(u16 hl) { return (hl >> 8); }
	constexpr static u8 low(u16 hl) { return hl & 0xFF; }
	constexpr static u16 to16(u8 h, u8 l) { return (h << 8) | l; }
	constexpr static void store16(u8& h, u8& l, u16 d) { h = high(d); l = low(d); }

	ReferenceData read(addr a);
	ReferenceData read(u8 a_h, u8 a_l) { return read(to16(a_h, a_l)); }
	ReferenceData readHL() { return read(H, L); }
	ReferenceData readN() { return read(PC++); }

	void loadZ() { Z = readN(); }

	void loadZW() {
		loadZ();
		W = readN();
	}
	
	void write(addr a, u8 data);
	void write(addr a, Register& data);
	void write(u8 a_h, u8 a_l, Register& data) { write(to16(a_h, a_l), data); }
	void writeHL(Register& data) { write(H, L, data); }

	void invalid();
	void cb();
	void alu();

	// Instructions
	void nop() {}
	void stop();

	// 16-bit load
	void push(u8 h, u8 l) {
		stepComponents();
		write(--SP, h);
		write(--SP, l);
	}

	void pop(Register& h, Register& l) {
		l = read(SP++);
		h = read(SP++);
	}

	void loadSP() {	
		loadZW();
		SP = (W << 8) | Z; 
	}

	// 8-bit load
	void load() {
		Register& dst = reg((IR >> 3) & 0x7);
		Register& src = reg(IR & 0x7);

		if (&src == &Z && (IR & 0x40)) Z = readHL();
		dst = src;
		if (&dst == &Z) writeHL(Z);
	}

	// rotate/shift/bit instructions
	void rotateLeft(Register& reg, bool circular = false) {
		bool c = (reg & 0x80);
		reg <<= 1;

		u32 src = reg.second;
		// hack to support sameboy bios
		if (!circular && F.C.second != 0) {
			reg.second = F.C.second;
		}

		reg |= u8((circular) ? c : F.C);

		F.Z = reg == 0;
		F.N = false;
		F.H = false;
		F.C = c;
		F.C.second = src;
	}

	void rotateRight(Register& reg, bool circular = false) {
		bool c = (reg & 0x1);
		reg >>= 1;
		reg |= ((circular) ? c : F.C) << 7;

		F.Z = reg == 0;
		F.N = false;
		F.H = false;
		F.C = c;
		F.C.second = reg.second;
	}

	void shiftLA(Register& reg) {
		F.C = reg & 0x80;
		F.C.second = reg.second;
		reg <<= 1;
		F.Z = reg == 0;
		F.H = false;
		F.N = false;
	}
	
	void shiftRA(Register& reg) {
		F.C = reg & 0x1;
		F.C.second = reg.second;
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

	void shiftRL(Register& reg) {
		F.C = reg & 0x1;
		F.C.second = reg.second;
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
		A = u8(~A);
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

	void _add(Register& r, u8 val, bool carry = false) {
		u16 res = r + val + carry;
		F.H = (r ^ val ^ res) & 0x10;
		F.C = res > 0xFF;
		r = u8(res);
		F.N = false;
	}

	void _sub(Register& r, u8 val, bool carry = false) {
		s16 res = r - val - carry;
		F.H = (r ^ val ^ res) & 0x10;
		F.C = res < 0;
		r = u8(res);
		F.N = true;
	}

	void inc() {
		Register& r = reg((IR >> 3) & 0x7);
		bool isZ = &r == &Z;
		if (isZ) Z = readHL();

		F.H = ((r & 0xf) + 1) > 0xf;
		++r;
		F.Z = r == 0;
		F.N = false;

		if (isZ) writeHL(Z);
	}

	void dec() { 
		Register& r = reg((IR >> 3) & 0x7);
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
		loadZ();

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
