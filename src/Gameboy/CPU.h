#pragma once

//need to clean up these includes
#include <string>
#include <string_view>
#include <cstdio>
#include <cstdarg>

#include "Util/Types.h"

class Memory;
class Scheduler;
class Interrupt;

#define Carry			0x10
#define HalfCarry		0x20
#define Negative		0x40
#define Zero			0x80

struct Flags {
	u8 : 4;
	u8 C : 1;
	u8 HC : 1;
	u8 N : 1;
	u8 Z : 1;
};

struct CPU {
	Memory& mem;
	Scheduler& scheduler;
	Interrupt& interrupt;

	union {
		struct {
			u8 low;
			u8 high;
		};
		struct {
			u16 value;
		};
	} bc{}, de{}, hl{};

	union {
		struct {
			Flags low;
			u8 high;
		};
		struct {
			u16 value;
		};
	} af{};

	u16& AF = af.value; u8& A = af.high; Flags& F = af.low;
	u16& BC = bc.value; u8& B = bc.high; u8& C = bc.low;
	u16& DE = de.value; u8& D = de.high; u8& E = de.low;
	u16& HL = hl.value; u8& H = hl.high; u8& L = hl.low;

	u16 PC, SP;
	u8 opcode;
	bool IME = false;

	CPU(Memory&, Scheduler&, Interrupt&);
	void handlePrint();
	bool handleInterrupts();
	void clean();
	void ExecuteOpcode();

private:
	bool haltBug = false;
	bool lowPower = false;
	bool handler = false;

	void fireInterrupt(u8 interrupt);

	void write(u16 loc, u8 value);
	void write(u16 loc, u16 value);

	/*
	template<typename T>
	void write(u16 loc, T value) {
		u8 amount = sizeof(T);

		cycles += amount;

		while (amount > 0) {
			gb.mem.Write(loc++, (value >> (8 * (sizeof(T) - amount--))) & 0xFF);
		}
	}
	*/

	u8 FetchOpcode(u16);
	bool CheckZero();
	bool CheckNegative();
	bool CheckHalfCarry();
	bool CheckCarry();
	
	u8 SetFlags(u16 flags, u16 ans, u8 old = 0, u8 diff = 0);
	u8 SetCarry(u16 ans);
	u8 SetHalfCarry(u8 ans, u8 old, u8 diff);
	//u16 SetNegative(u16 ans = 0); no real need for this /shrug
	u8 SetZero(int ans);
	inline void SetCarry(bool val);
	inline void SetHalfCarry(bool val);
	inline void SetNegative(bool val);
	inline void SetZero(bool val);

	template <typename T>
	T GetLEBytes();

	template <typename T>
	T GetLEBytes(u16 addr);

	template <typename T>
	T GetLEBytes(u16& addr, bool increase);

	//ALU
	void Add(u16 in);
	void Add(u8 in, bool carry = false);
	void Sub(u8 in, bool carry = false);
	void And(u8 in);
	void Xor(u8 in);
	void Or(u8 in);
	void Compare(u8 in);
	void Increase(u8& reg);
	void Decrease(u8& reg);
	void Increase(u16& reg);
	void Decrease(u16& reg);

	//CB
	template<typename T, typename... Args>
	u8 M_Write_Helper(T func, Args... args);

	void handleCB();
	void RotateLeft(u8& reg, bool carry = false);
	void RotateRight(u8& reg, bool carry = false);
	void ShiftLeftArithmetic(u8&);
	void ShiftRightArithmetic(u8&);
	void Swap(u8&);
	void ShiftRightLogical(u8&);
	void Bit(u8 reg, u8 bit);
	void Set(u8& reg, u8 bit);
	void Reset(u8& reg, u8 bit);

	//Misc
	void Load(u8& loc, u8 val);
	void Load(u16& loc, u16 val);
	void Push(u16& reg_pair);
	void Pop(u16& reg_pair);
	void Jump(u16 loc, bool cond = true);
	void JumpRelative(s8 offset, bool cond = true);
	void Call(u16 loc, bool cond = true);
	void Ret();
	void Rst(u8);
};