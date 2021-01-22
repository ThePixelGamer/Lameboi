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

	u8 FetchOpcode(u16);
	bool CheckZero();
	bool CheckNegative();
	bool CheckHalfCarry();
	bool CheckCarry();
	
	u8 updateFlags(u16 flags, u16 ans, u8 old = 0, u8 diff = 0);
	u8 updateCarry(u16 ans);
	u8 updateHalfCarry(u8 ans, u8 old, u8 diff);
	u8 updateZero(int ans);
	void setCarry(bool val);
	void setHalfCarry(bool val);
	void setNegative(bool val);
	void setZero(bool val);

	template <typename T>
	T getLEBytes(u16& addr, bool increase);

	// get the next PC byte(s)
	template <typename T>
	T nextBytes();

	// read byte(s) from bus[addr]
	template <typename T>
	T readBytes(u16 addr);

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
	void Push(u16 reg_pair);
	void Pop(u16& reg_pair);
	void Jump(u16 loc, bool cond = true);
	void JumpRelative(s8 offset, bool cond = true);
	void Call(u16 loc, bool cond = true);
	void Ret();
	void Rst(u8);
};