#pragma once
#include <string>
#include <string_view>
#include <stdio.h>
#include <stdarg.h>

#include "Memory.h"

struct Gameboy;

#define Carry			0x1
#define HalfCarry		0x2
#define Negative		0x4
#define Zero			0x8
#define Carry_Set		0x10
#define HalfCarry_Set	0x20
#define Negative_Set	0x40
#define Zero_Set		0x80
#define Carry_Unset		0x100
#define HalfCarry_Unset 0x200
#define Negative_Unset	0x400
#define Zero_Unset		0x800

struct individual {
    u8 low;
    u8 high;
};
struct together {
    u16 value;
};
union rp {
    individual S;
    together P;
};
struct instruction {
  std::string_view name;
  int arg;
};

struct CPU {
	Gameboy& gb;

	rp af{};
	rp bc{};
	rp de{};
	rp hl{};

	u8& A = af.S.high; u8& F = af.S.low;
	u8& B = bc.S.high; u8& C = bc.S.low;
	u8& D = de.S.high; u8& E = de.S.low;
	u8& H = hl.S.high; u8& L = hl.S.low;

	u16& AF = af.P.value;
	u16& BC = bc.P.value;
	u16& DE = de.P.value;
	u16& HL = hl.P.value;

	u16 PC, SP;
	u8 opcode;

	CPU(Gameboy&);
	void handlePrint();
	void Clean();
	void ExecuteOpcode();
	u8 FetchOpcode(u16);
	bool CheckZero();
	bool CheckNegative();
	bool CheckHalfCarry();
	bool CheckCarry();
	u16 setFlags(u16 flags, u16 ans = 0, u8 old = 0, u8 diff = 0);

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
	void handleCB();
	void RotateLeft(u8& reg, bool carry = false);
	void RotateRight(u8& reg, bool carry = false);
	void ShiftLeftArithmetic(u8&);
	void ShiftRightArithmetic(u8&);
	void Swap(u8&);
	void ShiftRightLogical(u8&);
	void Bit(u8 bit, u8 reg);
	void Set(u8 bit, u8& reg);
	void Reset(u8 bit, u8& reg);

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