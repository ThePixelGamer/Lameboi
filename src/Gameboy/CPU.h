#pragma once

//need to clean up these includes
#include <string>
#include <string_view>
#include <cstdio>
#include <cstdarg>

#include "Util/RegPair.h"
#include "Util/Types.h"

class Memory;
class Scheduler;
class Interrupt;

#define Carry			0x10
#define HalfCarry		0x20
#define Zero			0x80

struct Flags {
	bool Z = false;
	bool N = false;
	bool HC = false;
	bool C = false;

	operator u8() const {
		return (Z << 7) | (N << 6) | (HC << 5) | (C << 4);
	}

	void operator=(u8 val) {
		Z = val & 0x80;
		N = val & 0x40;
		HC = val & 0x20;
		C = val & 0x10;
	}
};

struct CPU {
	Memory& mem;
	Scheduler& scheduler;
	Interrupt& interrupt;

	RegPair<u8, Flags> AF; u8& A = AF.high; Flags& F = AF.low;
	RegPair<u8, u8> BC;	   u8& B = BC.high; u8& C = BC.low;
	RegPair<u8, u8> DE;	   u8& D = DE.high; u8& E = DE.low;
	RegPair<u8, u8> HL;	   u8& H = HL.high; u8& L = HL.low;

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
	void Push(u16 reg_pair);

	template <typename R>
	void Pop(R& reg_pair);

	void Jump(u16 loc, bool cond = true);
	void JumpRelative(s8 offset, bool cond = true);
	void Call(u16 loc, bool cond = true);
	void Ret();
	void Rst(u8);
};