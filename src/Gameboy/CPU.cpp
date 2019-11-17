#include "CPU.h"

#include "Gameboy.h"

#include <stdexcept>
#include <array>
#include <intrin.h>
#include "fmt/core.h"
#include "fmt/printf.h"

std::array<instruction, 0x100> const instTable{{
//             0                              1                   2								3                 4                         5                       6			         7                     8                        9               A                     B                C                      D                       E                F
	{"NOP",						0}, {"LD BC, $%04hX",	3}, {"LD (BC), A",				0}, {"INC BC",		0}, {"INC B",			0}, {"DEC B",		0}, {"LD B, $%02hhX",    3}, {"RLCA",		0}, {"LD ($%04hX), SP",	3}, {"ADD HL, BC", 0}, {"LD A, (BC)",     0}, {"DEC BC",  0}, {"INC C",			 0}, {"DEC C",		 0}, {"LD C, $%02hhX", 3}, {"RRCA",    0}, //00
	{"STOP",					0}, {"LD DE, $%04hX",	3}, {"LD (DE), A",				0}, {"INC DE",		0}, {"INC D",			0}, {"DEC D",		0}, {"LD D, $%02hhX",    3}, {"RLA",		0}, {"JR $%02hhX",		3}, {"ADD HL, DE", 0}, {"LD A, (DE)",     0}, {"DEC DE",  0}, {"INC E",			 0}, {"DEC E",		 0}, {"LD E, $%02hhX", 3}, {"RRA",     0}, //10
	{"JR NZ, $%02hhX",			3}, {"LD HL, $%04hX",	3}, {"LD (HL+), A",				0}, {"INC HL",		0}, {"INC H",			0}, {"DEC H",		0}, {"LD H, $%02hhX",    3}, {"DAA",		0}, {"JR Z, $%02hhX",	3}, {"ADD HL, HL", 0}, {"LD A, (HL+)",    0}, {"DEC HL",  0}, {"INC L",			 0}, {"DEC L",		 0}, {"LD L, $%02hhX", 3}, {"CPL",     0}, //20
	{"JR NC, $%02hhX",			3}, {"LD SP, $%04hX",	3}, {"LD (HL-), A",				0}, {"INC SP",		0}, {"INC (HL)",		0}, {"DEC (HL)",	0}, {"LD (HL), $%02hhX", 3}, {"SCF",		0}, {"JR C, $%02hhX",	3}, {"ADD HL, SP", 0}, {"LD A, (HL-)",    0}, {"DEC SP",  0}, {"INC A",			 0}, {"DEC A",		 0}, {"LD A, $%02hhX", 3}, {"CCF",     0}, //30
	{"LD B, B",					0}, {"LD B, C",			0}, {"LD B, D",					0}, {"LD B, E",		0}, {"LD B, H",			0}, {"LD B, L",		0}, {"LD B, (HL)",		 0}, {"LD B, A",    0}, {"LD C, B",         0}, {"LD C, C",    0}, {"LD C, D",        0}, {"LD C, E", 0}, {"LD C, H",		 0}, {"LD C, L",	 0}, {"LD C, (HL)",    0}, {"LD C, A", 0}, //40
	{"LD D, B",					0}, {"LD D, C",			0}, {"LD D, D",					0}, {"LD D, E",		0}, {"LD D, H",			0}, {"LD D, L",		0}, {"LD D, (HL)",		 0}, {"LD D, A",    0}, {"LD E, B",         0}, {"LD E, C",    0}, {"LD E, D",        0}, {"LD E, E", 0}, {"LD E, H",		 0}, {"LD E, L",	 0}, {"LD E, (HL)",    0}, {"LD E, A", 0}, //50
	{"LD H, B",					0}, {"LD H, C",			0}, {"LD H, D",					0}, {"LD H, E",		0}, {"LD H, H",			0}, {"LD H, L",		0}, {"LD H, (HL)",		 0}, {"LD H, A",    0}, {"LD L, B",         0}, {"LD L, C",    0}, {"LD L, D",        0}, {"LD L, E", 0}, {"LD L, H",		 0}, {"LD L, L",	 0}, {"LD L, (HL)",    0}, {"LD L, A", 0}, //60
	{"LD (HL), B",				0}, {"LD (HL), C",		0}, {"LD (HL), D",				0}, {"LD (HL), E",	0}, {"LD (HL), H",		0}, {"LD (HL), L",	0}, {"HALT",			 0}, {"LD (HL), A", 0}, {"LD A, B",         0}, {"LD A, C",    0}, {"LD A, D",        0}, {"LD A, E", 0}, {"LD A, H",		 0}, {"LD A, L",	 0}, {"LD A, (HL)",    0}, {"LD A, A", 0}, //70
	{"ADD B",					0}, {"ADD C",			0}, {"ADD D",					0}, {"ADD E",		0}, {"ADD H",			0}, {"ADD L",		0}, {"ADD (HL)",		 0}, {"ADD A",		0}, {"ADC B",			0}, {"ADC C",	   0}, {"ADC D",		  0}, {"ADC E",   0}, {"ADC H",			 0}, {"ADC L",		 0}, {"ADC (HL)",	   0}, {"ADC A",   0}, //80
	{"SUB B",					0}, {"SUB C",			0}, {"SUB D",					0}, {"SUB E",		0}, {"SUB H",			0}, {"SUB L",		0}, {"SUB (HL)",		 0}, {"SUB A",		0}, {"SBC B",			0}, {"SBC C",	   0}, {"SBC D",		  0}, {"SBC E",   0}, {"SBC H",			 0}, {"SBC L",		 0}, {"SBC (HL)",	   0}, {"SBC A",   0}, //90
	{"AND B",					0}, {"AND C",			0}, {"AND D",					0}, {"AND E",		0}, {"AND H",			0}, {"AND L",		0}, {"AND (HL)",		 0}, {"AND A",		0}, {"XOR B",			0}, {"XOR C",	   0}, {"XOR D",		  0}, {"XOR E",   0}, {"XOR H",			 0}, {"XOR L",		 0}, {"XOR (HL)",	   0}, {"XOR A",   0}, //a0
	{"OR B",					0}, {"OR C",			0}, {"OR D",					0}, {"OR E",		0}, {"OR H",			0}, {"OR L",		0}, {"OR (HL)",			 0}, {"OR A",		0}, {"CP B",			0}, {"CP C",	   0}, {"CP D",			  0}, {"CP E",    0}, {"CP H",			 0}, {"CP L",		 0}, {"CP (HL)",	   0}, {"CP A",    0}, //b0
	{"RET NZ",					0}, {"POP BC",			0}, {"JP NZ, $%04hX",			3}, {"JP $%04hX",	3}, {"CALL NZ, $%04hX",	3}, {"PUSH BC",		0}, {"ADD A, $%02hhX",	 3}, {"RST $00",	0}, {"RET Z",			0}, { "RET",       0}, {"JP Z, $hX",	  3}, {"CB:",     0}, {"CALL Z, $%04hX", 3}, {"CALL $%04hX", 3}, {"ADC $%02hhX",   3}, {"RST $08", 0}, //c0
	{"RET NC",					0}, {"POP DE",			0}, {"JP NC, $%04hX",			3}, {"",			0}, {"CALL NC, $%04hX",	3}, {"PUSH DE",		0}, {"SUB $%02hhX",		 3}, {"RST $10",	0}, {"RET C",			0}, { "RETI",      0}, {"JP C, $hX",	  3}, {"",        0}, {"CALL C, $%04hX", 3}, { "",			 0}, {"SBC $%02hhX",   3}, {"RST $18", 0}, //d0
	{"LD ($%04hX+$%02hhX), A",	2}, {"POP HL",			0}, {"LD ($%04hX+$%02hhX), A",	1}, {"",			0}, {"",				0}, {"PUSH HL",		0}, {"AND $%02hhX",		 3}, {"RST $20",	0}, {"ADD SP,%hhd",		3}, { "JP HL",     0}, {"LD ($%02hX), A", 3}, {"",        0}, {"",				 0}, { "",			 0}, {"XOR $%02hhX",   3}, {"RST $28", 0}, //e0
	{"LD A, ($%04hX+$%02hhX)",	2}, {"POP AF",			0}, {"LD A, ($%04hX+$%02hhX)",	1}, {"DI",			0}, {"",				0}, {"PUSH AF",		0}, {"OR $%02hhX",		 3}, {"RST $30",	0}, {"LD HL, SP+%hhd",	3}, {"LD SP,HL",   0}, {"LD A, ($%02hX)", 3}, {"EI",      0}, {"",				 0}, { "",			 0}, {"CP $%02hhX",    3}, {"RST $38", 0}, //f0
}};
std::array<instruction, 0x100> const CBTable{{
//     0               1               2                3              4                 5                6                   7                8               9               A              B                C              D                   E                 F
	{"RLC B",	0}, {"RLC C",	0}, {"RLC D",	0}, {"RLC E",	0}, {"RLC H",	 0}, {"RLC L",	  0}, {"RLC (HL)",    0}, {"RLC A",    0}, {"RRC B",   0}, {"RRC C",   0}, {"RRC D",   0}, {"RRC E",   0}, {"RRC H",	0}, {"RRC L",	 0}, {"RRC (HL)",    0}, {"RRC A",    0}, //00
	{"RL B",	0}, {"RL C",	0}, {"RL D",	0}, {"RL E",	0}, {"RL H",	 0}, {"RL L",	  0}, {"RL (HL)",     0}, {"RL A",     0}, {"RR B",	   0}, {"RR C",	   0}, {"RR D",    0}, {"RR E",	   0}, {"RR H",		0}, {"RR L",	 0}, {"RR (HL)",     0}, {"RR A",     0}, //10
	{"SLA B",	0}, {"SLA C",	0}, {"SLA D",	0}, {"SLA E",	0}, {"SLA H",	 0}, {"SLA L",	  0}, {"SLA (HL)",    0}, {"SLA A",    0}, {"SRA B",   0}, {"SRA C",   0}, {"SRA D",   0}, {"SRA E",   0}, {"SRA H",	0}, {"SRA L",	 0}, {"SRA (HL)",    0}, {"SRA A",    0}, //20
	{"SWAP B",	0}, {"SWAP C",	0}, {"SWAP D",	0}, {"SWAP E",	0}, {"SWAP H",	 0}, {"SWAP L",   0}, {"SWAP (HL)",   0}, {"SWAP A",   0}, {"SRL B",   0}, {"SRL C",   0}, {"SRL D",   0}, {"SRL E",   0}, {"SRL H",	0}, {"SRL L",	 0}, {"SRL (HL)",    0}, {"SRL A",    0}, //30
	{"BIT 0,B",	0}, {"BIT 0,C",	0}, {"BIT 0,C",	0}, {"BIT 0,C",	0}, {"BIT 0, H", 0}, {"BIT 0, L", 0}, {"BIT 0, (HL)", 0}, {"BIT 0, A", 0}, {"BIT 1,B", 0}, {"BIT 1,C", 0}, {"BIT 1,C", 0}, {"BIT 1,C", 0}, {"BIT 1, H",	0}, {"BIT 1, L", 0}, {"BIT 1, (HL)", 0}, {"BIT 1, A", 0}, //40
	{"BIT 2,B",	0}, {"BIT 2,C",	0}, {"BIT 2,C",	0}, {"BIT 2,C",	0}, {"BIT 2, H", 0}, {"BIT 2, L", 0}, {"BIT 2, (HL)", 0}, {"BIT 2, A", 0}, {"BIT 3,B", 0}, {"BIT 3,C", 0}, {"BIT 3,C", 0}, {"BIT 3,C", 0}, {"BIT 3, H",	0}, {"BIT 3, L", 0}, {"BIT 3, (HL)", 0}, {"BIT 3, A", 0}, //50
	{"BIT 4,B",	0}, {"BIT 4,C",	0}, {"BIT 4,C",	0}, {"BIT 4,C",	0}, {"BIT 4, H", 0}, {"BIT 4, L", 0}, {"BIT 4, (HL)", 0}, {"BIT 4, A", 0}, {"BIT 5,B", 0}, {"BIT 5,C", 0}, {"BIT 5,C", 0}, {"BIT 5,C", 0}, {"BIT 5, H",	0}, {"BIT 5, L", 0}, {"BIT 5, (HL)", 0}, {"BIT 5, A", 0}, //60
	{"BIT 6,B",	0}, {"BIT 6,C",	0}, {"BIT 6,C",	0}, {"BIT 6,C",	0}, {"BIT 6, H", 0}, {"BIT 6, L", 0}, {"BIT 6, (HL)", 0}, {"BIT 6, A", 0}, {"BIT 7,B", 0}, {"BIT 7,C", 0}, {"BIT 7,C", 0}, {"BIT 7,C", 0}, {"BIT 7, H",	0}, {"BIT 7, L", 0}, {"BIT 7, (HL)", 0}, {"BIT 7, A", 0}, //70
	{"RES 0,B",	0}, {"RES 0,C",	0}, {"RES 0,C",	0}, {"RES 0,C",	0}, {"RES 0, H", 0}, {"RES 0, L", 0}, {"RES 0, (HL)", 0}, {"RES 0, A", 0}, {"RES 1,B", 0}, {"RES 1,C", 0}, {"RES 1,C", 0}, {"RES 1,C", 0}, {"RES 1, H",	0}, {"RES 1, L", 0}, {"RES 1, (HL)", 0}, {"RES 1, A", 0}, //80
	{"RES 2,B",	0}, {"RES 2,C",	0}, {"RES 2,C",	0}, {"RES 2,C",	0}, {"RES 2, H", 0}, {"RES 2, L", 0}, {"RES 2, (HL)", 0}, {"RES 2, A", 0}, {"RES 3,B", 0}, {"RES 3,C", 0}, {"RES 3,C", 0}, {"RES 3,C", 0}, {"RES 3, H",	0}, {"RES 3, L", 0}, {"RES 3, (HL)", 0}, {"RES 3, A", 0}, //90
	{"RES 4,B",	0}, {"RES 4,C",	0}, {"RES 4,C",	0}, {"RES 4,C",	0}, {"RES 4, H", 0}, {"RES 4, L", 0}, {"RES 4, (HL)", 0}, {"RES 4, A", 0}, {"RES 5,B", 0}, {"RES 5,C", 0}, {"RES 5,C", 0}, {"RES 5,C", 0}, {"RES 5, H",	0}, {"RES 5, L", 0}, {"RES 5, (HL)", 0}, {"RES 5, A", 0}, //A0
	{"RES 6,B",	0}, {"RES 6,C",	0}, {"RES 6,C",	0}, {"RES 6,C",	0}, {"RES 6, H", 0}, {"RES 6, L", 0}, {"RES 6, (HL)", 0}, {"RES 6, A", 0}, {"RES 7,B", 0}, {"RES 7,C", 0}, {"RES 7,C", 0}, {"RES 7,C", 0}, {"RES 7, H",	0}, {"RES 7, L", 0}, {"RES 7, (HL)", 0}, {"RES 7, A", 0}, //B0
	{"SET 0,B",	0}, {"SET 0,C",	0}, {"SET 0,C",	0}, {"SET 0,C",	0}, {"SET 0, H", 0}, {"SET 0, L", 0}, {"SET 0, (HL)", 0}, {"SET 0, A", 0}, {"SET 1,B", 0}, {"SET 1,C", 0}, {"SET 1,C", 0}, {"SET 1,C", 0}, {"SET 1, H",	0}, {"SET 1, L", 0}, {"SET 1, (HL)", 0}, {"SET 1, A", 0}, //C0
	{"SET 2,B",	0}, {"SET 2,C",	0}, {"SET 2,C",	0}, {"SET 2,C",	0}, {"SET 2, H", 0}, {"SET 2, L", 0}, {"SET 2, (HL)", 0}, {"SET 2, A", 0}, {"SET 3,B", 0}, {"SET 3,C", 0}, {"SET 3,C", 0}, {"SET 3,C", 0}, {"SET 3, H",	0}, {"SET 3, L", 0}, {"SET 3, (HL)", 0}, {"SET 3, A", 0}, //D0
	{"SET 4,B",	0}, {"SET 4,C",	0}, {"SET 4,C",	0}, {"SET 4,C",	0}, {"SET 4, H", 0}, {"SET 4, L", 0}, {"SET 4, (HL)", 0}, {"SET 4, A", 0}, {"SET 5,B", 0}, {"SET 5,C", 0}, {"SET 5,C", 0}, {"SET 5,C", 0}, {"SET 5, H",	0}, {"SET 5, L", 0}, {"SET 5, (HL)", 0}, {"SET 5, A", 0}, //E0
	{"SET 6,B",	0}, {"SET 6,C",	0}, {"SET 6,C",	0}, {"SET 6,C",	0}, {"SET 6, H", 0}, {"SET 6, L", 0}, {"SET 6, (HL)", 0}, {"SET 6, A", 0}, {"SET 7,B", 0}, {"SET 7,C", 0}, {"SET 7,C", 0}, {"SET 7,C", 0}, {"SET 7, H",	0}, {"SET 7, L", 0}, {"SET 7, (HL)", 0}, {"SET 7, A", 0}, //F0
}};

void CPU::handlePrint() {
	/*fprintf(gb.log, "[$%04X] ", PC-1); //PSI

	instruction inst = (opcode == 0xCB) ? CBTable[FetchOpcode(PC)] : instTable[opcode];
	auto qwqs = std::string(inst.name);
	auto qwq = qwqs.c_str();
	switch(inst.arg) {
		case 0: fprintf(gb.log, qwq); break;
		case 1: fprintf(gb.log, qwq, 0xFF00, C); break;
		case 2: fprintf(gb.log, qwq, 0xFF00, GetLEBytes<u8>(PC)); break;
		case 3: fprintf(gb.log, qwq, GetLEBytes<u16>(PC)); break;
	}*/

	//fprintf(gb.log, "PC:%04x OPC:%02x %02x %02x C:%x H:%x N:%x Z:%x A:%02x\n", PC - 1, opcode, GetLEBytes<u8>(PC), GetLEBytes<u8>(PC + 1), CheckCarry(), CheckHalfCarry(), CheckNegative(), CheckZero(), A); //LilaQ
}

CPU::CPU(Gameboy& t_gb) : gb(t_gb) {
	Clean();	
}

void CPU::Clean() {
	AF = 0;
	BC = 0;
	DE = 0;
	HL = 0;
	SP = 0;
	PC = 0;
	opcode = 0;
	gb.IME = false;
}

void CPU::ExecuteOpcode() {
	opcode = FetchOpcode(PC++);

	handlePrint();

	switch(opcode) {
		case 0x03: Increase(BC); break; //INC BC
		case 0x13: Increase(DE); break; //INC DE
		case 0x23: Increase(HL); break; //INC HL
		case 0x33: Increase(SP); break; //INC SP
		case 0x0B: Decrease(BC); break; //DEC BC
		case 0x1B: Decrease(DE); break; //DEC DE
		case 0x2B: Decrease(HL); break; //DEC HL
		case 0x3B: Decrease(SP); break; //DEC SP

		case 0x04: Increase(B); break; //INC B
		case 0x0C: Increase(C); break; //INC C
		case 0x14: Increase(D); break; //INC D
		case 0x1C: Increase(E); break; //INC E
		case 0x24: Increase(H); break; //INC H
		case 0x2C: Increase(L); break; //INC L
		case 0x34: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Increase(value); return value;}()); break; //INC (HL)
		case 0x3C: Increase(A); break;

		case 0x05: Decrease(B); break; //DEC B
		case 0x0D: Decrease(C); break; //DEC C
		case 0x15: Decrease(D); break; //DEC D
		case 0x1D: Decrease(E); break; //DEC E
		case 0x25: Decrease(H); break; //DEC H
		case 0x2D: Decrease(L); break; //DEC L
		case 0x35: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Decrease(value); return value;}()); break; //DEC (HL)		
		case 0x3D: Decrease(A); break;
			
		case 0x01: Load(BC, GetLEBytes<u16>()); break; //LD BC,u16
		case 0x11: Load(DE, GetLEBytes<u16>()); break; //LD DE,u16
		case 0x21: Load(HL, GetLEBytes<u16>()); break; //LD HL,u16
		case 0x31: Load(SP, GetLEBytes<u16>()); break; //LD SP,u16
		case 0x08: gb.mem.Write(GetLEBytes<u16>(), SP); break; //LD (u16),SP
			
		case 0x02: gb.mem.Write(BC, A); break; //LD (BC),A
		case 0x12: gb.mem.Write(DE, A); break; //LD (DE),A
		case 0x22: gb.mem.Write(HL++, A); break; //LD (HL+),A
		case 0x32: gb.mem.Write(HL--, A); break; //LD (HL-),A

		case 0x06: Load(B, GetLEBytes<u8>()); break; //LD B,u8
		case 0x0E: Load(C, GetLEBytes<u8>()); break; //LD C,u8
		case 0x16: Load(D, GetLEBytes<u8>()); break; //LD D,u8
		case 0x1E: Load(E, GetLEBytes<u8>()); break; //LD E,u8
		case 0x26: Load(H, GetLEBytes<u8>()); break; //LD H,u8
		case 0x2E: Load(L, GetLEBytes<u8>()); break; //LD L,u8
		case 0x36: gb.mem.Write(HL, GetLEBytes<u8>()); break; //LD (HL),u8
		case 0x3E: Load(A, GetLEBytes<u8>()); break; //LD A,u8

		case 0x0A: Load(A, GetLEBytes<u8>(BC)); break; //LD A,(BC)
		case 0x1A: Load(A, GetLEBytes<u8>(DE)); break; //LD A,(DE)
		case 0x2A: Load(A, GetLEBytes<u8>(HL++)); break; //LD A,(HL+)
		case 0x3A: Load(A, GetLEBytes<u8>(HL--)); break; //LD A,(HL-)
			
		case 0x09: Add(BC); break; //ADD HL,BC
		case 0x19: Add(DE); break; //ADD HL,DE
		case 0x29: Add(HL); break; //ADD HL,HL
		case 0x39: Add(SP); break; //ADD HL,SP

		case 0x07: RotateLeft(A); setFlags(Zero_Unset); break; //RLCA
		case 0x0F: RotateRight(A); setFlags(Zero_Unset); break;  //RRCA
		case 0x17: RotateLeft(A, true); setFlags(Zero_Unset); break; //RLA
		case 0x1F: RotateRight(A, true); setFlags(Zero_Unset); break; //RRA

		case 0x18: JumpRelative(GetLEBytes<u8>()); break; //JR i8
		case 0x20: JumpRelative(GetLEBytes<u8>(), !CheckZero()); break; //JR NZ,i8
		case 0x30: JumpRelative(GetLEBytes<u8>(), !CheckCarry()); break;
		case 0x28: JumpRelative(GetLEBytes<u8>(), CheckZero()); break;
		case 0x38: JumpRelative(GetLEBytes<u8>(), CheckCarry()); break;

		case 0x00: break; //NOP
		case 0x10: /*throw "Stop called";*/ break; //STOP

		case 0x27: { //DAA
			// note: assumes a is a uint8_t and wraps from 0xff to 0
			if (!CheckNegative()) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
				if (CheckCarry() || A > 0x99) { A += 0x60; setFlags(Carry_Set); }
				if (CheckHalfCarry() || (A & 0x0f) > 0x09) { A += 0x6; }
			} 
			else {  // after a subtraction, only adjust if (half-)carry occurred
				if (CheckCarry()) { A -= 0x60; }
				if (CheckHalfCarry()) { A -= 0x6; }
			}
			// these flags are always updated
			setFlags(((A == 0) ? Zero_Set : Zero_Unset)); // the usual z flag
			setFlags(HalfCarry_Unset); // h flag is always cleared
		} break;

		case 0x2F: { //CPL
			A = ~A; setFlags(Negative_Set | HalfCarry_Set);
		} break;

		case 0x37: setFlags(Carry_Set | Negative_Unset | HalfCarry_Unset); break; //SCF
		case 0x3F: setFlags((CheckCarry() ? Carry_Unset : Carry_Set) | Negative_Unset | HalfCarry_Unset); break; //CCF

		case 0x40: Load(B, B); break;
		case 0x41: Load(B, C); break;
		case 0x42: Load(B, D); break;
		case 0x43: Load(B, E); break;
		case 0x44: Load(B, H); break;
		case 0x45: Load(B, L); break;
		case 0x46: Load(B, GetLEBytes<u8>(HL)); break;
		case 0x47: Load(B, A); break;
		case 0x48: Load(C, B); break;
		case 0x49: Load(C, C); break;
		case 0x4A: Load(C, D); break;
		case 0x4B: Load(C, E); break;
		case 0x4C: Load(C, H); break;
		case 0x4D: Load(C, L); break;
		case 0x4E: Load(C, GetLEBytes<u8>(HL)); break;
		case 0x4F: Load(C, A); break;
		case 0x50: Load(D, B); break;
		case 0x51: Load(D, C); break;
		case 0x52: Load(D, D); break;
		case 0x53: Load(D, E); break;
		case 0x54: Load(D, H); break;
		case 0x55: Load(D, L); break;
		case 0x56: Load(D, GetLEBytes<u8>(HL)); break;
		case 0x57: Load(D, A); break;
		case 0x58: Load(E, B); break;
		case 0x59: Load(E, C); break;
		case 0x5A: Load(E, D); break;
		case 0x5B: Load(E, E); break;
		case 0x5C: Load(E, H); break;
		case 0x5D: Load(E, L); break;
		case 0x5E: Load(E, GetLEBytes<u8>(HL)); break;
		case 0x5F: Load(E, A); break;
		case 0x60: Load(H, B); break;
		case 0x61: Load(H, C); break;
		case 0x62: Load(H, D); break;
		case 0x63: Load(H, E); break;
		case 0x64: Load(H, H); break;
		case 0x65: Load(H, L); break;
		case 0x66: Load(H, GetLEBytes<u8>(HL)); break;
		case 0x67: Load(H, A); break;
		case 0x68: Load(L, B); break;
		case 0x69: Load(L, C); break;
		case 0x6A: Load(L, D); break;
		case 0x6B: Load(L, E); break;
		case 0x6C: Load(L, H); break;
		case 0x6D: Load(L, L); break;
		case 0x6E: Load(L, GetLEBytes<u8>(HL)); break;
		case 0x6F: Load(L, A); break;
		case 0x70: gb.mem.Write(HL, B); break;
		case 0x71: gb.mem.Write(HL, C); break;
		case 0x72: gb.mem.Write(HL, D); break;
		case 0x73: gb.mem.Write(HL, E); break;
		case 0x74: gb.mem.Write(HL, H); break;
		case 0x75: gb.mem.Write(HL, L); break;
		case 0x76: /*HALT*/ break;
		case 0x77: gb.mem.Write(HL, A); break;
		case 0x78: Load(A, B); break;
		case 0x79: Load(A, C); break;
		case 0x7A: Load(A, D); break;
		case 0x7B: Load(A, E); break;
		case 0x7C: Load(A, H); break;
		case 0x7D: Load(A, L); break;
		case 0x7E: Load(A, GetLEBytes<u8>(HL)); break;
		case 0x7F: Load(A, A); break;

		case 0x80: Add(B); break;
		case 0x81: Add(C); break;
		case 0x82: Add(D); break;
		case 0x83: Add(E); break;
		case 0x84: Add(H); break;
		case 0x85: Add(L); break;
		case 0x86: Add(GetLEBytes<u8>(HL)); break;
		case 0x87: Add(A); break; //ADD A, A
		case 0xC6: Add(GetLEBytes<u8>()); break; //ADD A,u8
		case 0x88: Add(B, CheckCarry()); break;
		case 0x89: Add(C, CheckCarry()); break;
		case 0x8A: Add(D, CheckCarry()); break;
		case 0x8B: Add(E, CheckCarry()); break;
		case 0x8C: Add(H, CheckCarry()); break;
		case 0x8D: Add(L, CheckCarry()); break;
		case 0x8E: Add(GetLEBytes<u8>(HL), CheckCarry()); break;
		case 0x8F: Add(A, CheckCarry()); break; //ADC A, A
		case 0xCE: Add(GetLEBytes<u8>(), CheckCarry()); break; //ADC A,u8

		case 0x90: Sub(B); break;
		case 0x91: Sub(C); break;
		case 0x92: Sub(D); break;
		case 0x93: Sub(E); break;
		case 0x94: Sub(H); break;
		case 0x95: Sub(L); break;
		case 0x96: Sub(GetLEBytes<u8>(HL)); break;
		case 0x97: Sub(A); break; //SUB A, A
		case 0xD6: Sub(GetLEBytes<u8>()); break; //SUB A,u8
		case 0x98: Sub(B, CheckCarry()); break;
		case 0x99: Sub(C, CheckCarry()); break;
		case 0x9A: Sub(D, CheckCarry()); break;
		case 0x9B: Sub(E, CheckCarry()); break;
		case 0x9C: Sub(H, CheckCarry()); break;
		case 0x9D: Sub(L, CheckCarry()); break;
		case 0x9E: Sub(GetLEBytes<u8>(HL), CheckCarry()); break; //SBC A, (HL)
		case 0x9F: Sub(A, CheckCarry()); break; //SBC A, A
		case 0xDE: Sub(GetLEBytes<u8>(), CheckCarry()); break; //SBC A,u8

		case 0xA0: And(B); break;
		case 0xA1: And(C); break;
		case 0xA2: And(D); break;
		case 0xA3: And(E); break;
		case 0xA4: And(H); break;
		case 0xA5: And(L); break;
		case 0xA6: And(GetLEBytes<u8>(HL)); break;
		case 0xA7: And(A); break; //AND A, A
		case 0xE6: And(GetLEBytes<u8>()); break; //AND A,u8

		case 0xA8: Xor(B); break;
		case 0xA9: Xor(C); break;
		case 0xAA: Xor(D); break;
		case 0xAB: Xor(E); break;
		case 0xAC: Xor(H); break;
		case 0xAD: Xor(L); break;
		case 0xAE: Xor(GetLEBytes<u8>(HL)); break;
		case 0xAF: Xor(A); break; //XOR A, A
		case 0xEE: Xor(GetLEBytes<u8>()); break; //XOR A,u8

		case 0xB0: Or(B); break;
		case 0xB1: Or(C); break;
		case 0xB2: Or(D); break;
		case 0xB3: Or(E); break;
		case 0xB4: Or(H); break;
		case 0xB5: Or(L); break;
		case 0xB6: Or(GetLEBytes<u8>(HL)); break;
		case 0xB7: Or(A); break; //OR A, A
		case 0xF6: Or(GetLEBytes<u8>()); break; //OR A,u8

		case 0xB8: Compare(B); break;
		case 0xB9: Compare(C); break;
		case 0xBA: Compare(D); break;
		case 0xBB: Compare(E); break;
		case 0xBC: Compare(H); break;
		case 0xBD: Compare(L); break;
		case 0xBE: Compare(GetLEBytes<u8>(HL)); break;
		case 0xBF: Compare(A); break; //CP A, A
		case 0xFE: Compare(GetLEBytes<u8>()); break; //CP A,u8
			
		case 0xC9: Ret(); break; //RET
		case 0xC0: if(!CheckZero()) Ret(); break; //RET NZ
		case 0xD0: if(!CheckCarry()) Ret(); break; //RET NC
		case 0xC8: if(CheckZero()) Ret(); break; //RET Z
		case 0xD8: if(CheckCarry()) Ret(); break; //RET C
		case 0xD9: Ret(); gb.IME = true; break; //RETI
			
		case 0xC5: Push(BC); break; //PUSH BC
		case 0xD5: Push(DE); break; //PUSH DE
		case 0xE5: Push(HL); break; //PUSH HL
		case 0xF5: Push(AF); break; //PUSH AF

		case 0xC1: Pop(BC); break; //POP BC
		case 0xD1: Pop(DE); break; //POP DE
		case 0xE1: Pop(HL); break; //POP HL
		case 0xF1: Pop(AF); F &= 0xF0; break; //POP AF
			
		case 0xC3: Jump(GetLEBytes<u16>()); break; //JP u16
		case 0xC2: Jump(GetLEBytes<u16>(), !CheckZero()); break; //JP NZ, u16
		case 0xD2: Jump(GetLEBytes<u16>(), !CheckCarry()); break; //JP NC, u16
		case 0xCA: Jump(GetLEBytes<u16>(), CheckZero()); break; //JP Z, u16
		case 0xDA: Jump(GetLEBytes<u16>(), CheckCarry()); break; //JP C, u16
		case 0xE9: Jump(HL); break; //JP HL
			
		case 0xCD: Call(GetLEBytes<u16>()); break; //CALL
		case 0xC4: Call(GetLEBytes<u16>(), !CheckZero()); break; //CALL NZ,u16
		case 0xD4: Call(GetLEBytes<u16>(), !CheckCarry()); break; //CALL NC,u16
		case 0xCC: Call(GetLEBytes<u16>(), CheckZero()); break; //CALL Z,u16
		case 0xDC: Call(GetLEBytes<u16>(), CheckCarry()); break; //CALL C,u16

		case 0xC7: Rst(0x00); break; //RST 00h
		case 0xCF: Rst(0x08); break; //RST 08h
		case 0xD7: Rst(0x10); break; //RST 10h
		case 0xDF: Rst(0x18); break; //RST 18h
		case 0xE7: Rst(0x20); break; //RST 20h
		case 0xEF: Rst(0x28); break; //RST 28h
		case 0xF7: Rst(0x30); break; //RST 30h
		case 0xFF: Rst(0x38); break; //RST 38h

		case 0xE8: { //ADD SP,i8
			u8 offset = GetLEBytes<u8>();
			setFlags(Zero_Unset | Negative_Unset | HalfCarry | Carry, (SP & 0xff) + offset, u8(SP), offset);
			SP += s8(offset);
		} break;

		case 0xF8: { //LD HL,SP+i8
			u8 offset = GetLEBytes<u8>();
			setFlags(Zero_Unset | Negative_Unset | HalfCarry | Carry, (SP & 0xff) + offset, u8(SP), offset);
			Load(HL, SP+s8(offset));
		} break;

		case 0xF9: Load(SP, HL); break; //LD SP,HL			
		case 0xF0: Load(A, GetLEBytes<u8>(GetLEBytes<u8>() + 0xFF00)); break; //LD A,(FF00+u8)
		case 0xF2: Load(A, GetLEBytes<u8>(0xFF00 + C)); break; //LD A,(FF00+C)
		case 0xFA: Load(A, GetLEBytes<u8>(GetLEBytes<u16>())); break; //LD A,(u16)
		case 0xE0: gb.mem.Write(0xFF00 + GetLEBytes<u8>(), A); break; //LD (FF00+u8),A
		case 0xE2: gb.mem.Write(0xFF00 + C, A); break; //LD (FF00+C),A
		case 0xEA: gb.mem.Write(GetLEBytes<u16>(), A); break; //LD (u16),A

		case 0xF3: gb.IME = false; break; //DI
		case 0xFB: gb.IME = true; break; //EI
		case 0xCB: handleCB(); break;

		default: /*fmt::printf("Unimplemented Opcode: 0x%02X\n", opcode);*/ break;
	}
	
	if(PC == 0x100) gb.continue_trigger = false;

    //fprintf(gb.log, " AF: $%04X BC: $%04X DE: $%04X HL: $%04X\n", AF, BC, DE, HL);
}

void CPU::handleCB() {
	opcode = FetchOpcode(PC++);

	switch(opcode) {
		case 0x00: RotateLeft(B); break; //RLC B
		case 0x01: RotateLeft(C); break; //RLC C
		case 0x02: RotateLeft(D); break; //RLC D
		case 0x03: RotateLeft(E); break; //RLC E
		case 0x04: RotateLeft(H); break; //RLC H
		case 0x05: RotateLeft(L); break; //RLC L
		case 0x06: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); RotateLeft(value); return value;}()); break; //RLC (HL)
		case 0x07: RotateLeft(A); break; //RLC A
		case 0x10: RotateLeft(B, true); break; //RL B
		case 0x11: RotateLeft(C, true); break; //RL C
		case 0x12: RotateLeft(D, true); break; //RL D
		case 0x13: RotateLeft(E, true); break; //RL E
		case 0x14: RotateLeft(H, true); break; //RL H
		case 0x15: RotateLeft(L, true); break; //RL L
		case 0x16: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); RotateLeft(value, true); return value;}()); break; //RL (HL)
		case 0x17: RotateLeft(A, true); break; //RL A

		case 0x08: RotateRight(B); break; //RRC B
		case 0x09: RotateRight(C); break; //RRC C
		case 0x0A: RotateRight(D); break; //RRC D
		case 0x0B: RotateRight(E); break; //RRC E
		case 0x0C: RotateRight(H); break; //RRC H
		case 0x0D: RotateRight(L); break; //RRC L
		case 0x0E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); RotateRight(value); return value;}()); break; //RRC (HL)
		case 0x0F: RotateRight(A); break; //RRC A
		case 0x18: RotateRight(B, true); break; //RR B
		case 0x19: RotateRight(C, true); break; //RR C
		case 0x1A: RotateRight(D, true); break; //RR D
		case 0x1B: RotateRight(E, true); break; //RR E
		case 0x1C: RotateRight(H, true); break; //RR H
		case 0x1D: RotateRight(L, true); break; //RR L
		case 0x1E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); RotateRight(value, true); return value;}()); break; //RR (HL)
		case 0x1F: RotateRight(A, true); break; //RR A

		case 0x20: ShiftLeftArithmetic(B); break; //SLA B
		case 0x21: ShiftLeftArithmetic(C); break; //SLA C
		case 0x22: ShiftLeftArithmetic(D); break; //SLA D
		case 0x23: ShiftLeftArithmetic(E); break; //SLA E
		case 0x24: ShiftLeftArithmetic(H); break; //SLA H
		case 0x25: ShiftLeftArithmetic(L); break; //SLA L
		case 0x26: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); ShiftLeftArithmetic(value); return value;}()); break; //SLA (HL)
		case 0x27: ShiftLeftArithmetic(A); break; //SLA A

		case 0x28: ShiftRightArithmetic(B); break; //SRA B
		case 0x29: ShiftRightArithmetic(C); break; //SRA C
		case 0x2A: ShiftRightArithmetic(D); break; //SRA D
		case 0x2B: ShiftRightArithmetic(E); break; //SRA E
		case 0x2C: ShiftRightArithmetic(H); break; //SRA H
		case 0x2D: ShiftRightArithmetic(L); break; //SRA L
		case 0x2E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); ShiftRightArithmetic(value); return value;}()); break; //SRA (HL)
		case 0x2F: ShiftRightArithmetic(A); break; //SRA A
		
		case 0x30: Swap(B); break; //SWAP B
		case 0x31: Swap(C); break; //SWAP C
		case 0x32: Swap(D); break; //SWAP D
		case 0x33: Swap(E); break; //SWAP E
		case 0x34: Swap(H); break; //SWAP H
		case 0x35: Swap(L); break; //SWAP L
		case 0x36: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Swap(value); return value;}()); break; //SWAP (HL)
		case 0x37: Swap(A); break; //SWAP A

		case 0x38: ShiftRightLogical(B); break; //SRL B
		case 0x39: ShiftRightLogical(C); break; //SRL C
		case 0x3A: ShiftRightLogical(D); break; //SRL D
		case 0x3B: ShiftRightLogical(E); break; //SRL E
		case 0x3C: ShiftRightLogical(H); break; //SRL H
		case 0x3D: ShiftRightLogical(L); break; //SRL L
		case 0x3E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); ShiftRightLogical(value); return value;}()); break; //SRL (HL)
		case 0x3F: ShiftRightLogical(A); break; //SRL A

		case 0x40: Bit(0, B); break; //BIT 0,B
		case 0x41: Bit(0, C); break; //BIT 0,C
		case 0x42: Bit(0, D); break; //BIT 0,D
		case 0x43: Bit(0, E); break; //BIT 0,E
		case 0x44: Bit(0, H); break; //BIT 0,H
		case 0x45: Bit(0, L); break; //BIT 0,L
		case 0x46: Bit(0, GetLEBytes<u8>(HL)); break; //BIT 0,(HL)
		case 0x47: Bit(0, A); break; //BIT 0,A
		case 0x48: Bit(1, B); break; //BIT 1,B
		case 0x49: Bit(1, C); break; //BIT 1,C
		case 0x4A: Bit(1, D); break; //BIT 1,D
		case 0x4B: Bit(1, E); break; //BIT 1,E
		case 0x4C: Bit(1, H); break; //BIT 1,H
		case 0x4D: Bit(1, L); break; //BIT 1,L
		case 0x4E: Bit(1, GetLEBytes<u8>(HL)); break; //BIT 1,(HL)
		case 0x4F: Bit(1, A); break; //BIT 1,A
		case 0x50: Bit(2, B); break; //BIT 2,B
		case 0x51: Bit(2, C); break; //BIT 2,C
		case 0x52: Bit(2, D); break; //BIT 2,D
		case 0x53: Bit(2, E); break; //BIT 2,E
		case 0x54: Bit(2, H); break; //BIT 2,H
		case 0x55: Bit(2, L); break; //BIT 2,L
		case 0x56: Bit(2, GetLEBytes<u8>(HL)); break; //BIT 2,(HL)
		case 0x57: Bit(2, A); break; //BIT 2,A
		case 0x58: Bit(3, B); break; //BIT 3,B
		case 0x59: Bit(3, C); break; //BIT 3,C
		case 0x5A: Bit(3, D); break; //BIT 3,D
		case 0x5B: Bit(3, E); break; //BIT 3,E
		case 0x5C: Bit(3, H); break; //BIT 3,H
		case 0x5D: Bit(3, L); break; //BIT 3,L
		case 0x5E: Bit(3, GetLEBytes<u8>(HL)); break; //BIT 3,(HL)
		case 0x5F: Bit(3, A); break; //BIT 3,A
		case 0x60: Bit(4, B); break; //BIT 4,B
		case 0x61: Bit(4, C); break; //BIT 4,C
		case 0x62: Bit(4, D); break; //BIT 4,D
		case 0x63: Bit(4, E); break; //BIT 4,E
		case 0x64: Bit(4, H); break; //BIT 4,H
		case 0x65: Bit(4, L); break; //BIT 4,L
		case 0x66: Bit(4, GetLEBytes<u8>(HL)); break; //BIT 4,(HL)
		case 0x67: Bit(4, A); break; //BIT 4,A
		case 0x68: Bit(5, B); break; //BIT 5,B
		case 0x69: Bit(5, C); break; //BIT 5,C
		case 0x6A: Bit(5, D); break; //BIT 5,D
		case 0x6B: Bit(5, E); break; //BIT 5,E
		case 0x6C: Bit(5, H); break; //BIT 5,H
		case 0x6D: Bit(5, L); break; //BIT 5,L
		case 0x6E: Bit(5, GetLEBytes<u8>(HL)); break; //BIT 5,(HL)
		case 0x6F: Bit(5, A); break; //BIT 5,A
		case 0x70: Bit(6, B); break; //BIT 6,B
		case 0x71: Bit(6, C); break; //BIT 6,C
		case 0x72: Bit(6, D); break; //BIT 6,D
		case 0x73: Bit(6, E); break; //BIT 6,E
		case 0x74: Bit(6, H); break; //BIT 6,H
		case 0x75: Bit(6, L); break; //BIT 6,L
		case 0x76: Bit(6, GetLEBytes<u8>(HL)); break; //BIT 6,(HL)
		case 0x77: Bit(6, A); break; //BIT 6,A
		case 0x78: Bit(7, B); break; //BIT 7,B
		case 0x79: Bit(7, C); break; //BIT 7,C
		case 0x7A: Bit(7, D); break; //BIT 7,D
		case 0x7B: Bit(7, E); break; //BIT 7,E
		case 0x7C: Bit(7, H); break; //BIT 7,H
		case 0x7D: Bit(7, L); break; //BIT 7,L
		case 0x7E: Bit(7, GetLEBytes<u8>(HL)); break; //BIT 7,(HL)
		case 0x7F: Bit(7, A); break; //BIT 7,A

		case 0x80: Reset(0, B); break; //RES 0,B
		case 0x81: Reset(0, C); break; //RES 0,C
		case 0x82: Reset(0, D); break; //RES 0,D
		case 0x83: Reset(0, E); break; //RES 0,E
		case 0x84: Reset(0, H); break; //RES 0,H
		case 0x85: Reset(0, L); break; //RES 0,L
		case 0x86: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(0, value); return value;}()); break; //RES 0,(HL)
		case 0x87: Reset(0, A); break; //RES 0,A
		case 0x88: Reset(1, B); break; //RES 1,B
		case 0x89: Reset(1, C); break; //RES 1,C
		case 0x8A: Reset(1, D); break; //RES 1,D
		case 0x8B: Reset(1, E); break; //RES 1,E
		case 0x8C: Reset(1, H); break; //RES 1,H
		case 0x8D: Reset(1, L); break; //RES 1,L
		case 0x8E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(1, value); return value;}()); break; //RES 1,(HL)
		case 0x8F: Reset(1, A); break; //RES 1,A
		case 0x90: Reset(2, B); break; //RES 2,B
		case 0x91: Reset(2, C); break; //RES 2,C
		case 0x92: Reset(2, D); break; //RES 2,D
		case 0x93: Reset(2, E); break; //RES 2,E
		case 0x94: Reset(2, H); break; //RES 2,H
		case 0x95: Reset(2, L); break; //RES 2,L
		case 0x96: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(2, value); return value;}()); break; //RES 2,(HL)
		case 0x97: Reset(2, A); break; //RES 2,A
		case 0x98: Reset(3, B); break; //RES 3,B
		case 0x99: Reset(3, C); break; //RES 3,C
		case 0x9A: Reset(3, D); break; //RES 3,D
		case 0x9B: Reset(3, E); break; //RES 3,E
		case 0x9C: Reset(3, H); break; //RES 3,H
		case 0x9D: Reset(3, L); break; //RES 3,L
		case 0x9E: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(3, value); return value;}()); break; //RES 3,(HL)
		case 0x9F: Reset(3, A); break; //RES 3,A
		case 0xA0: Reset(4, B); break; //RES 4,B
		case 0xA1: Reset(4, C); break; //RES 4,C
		case 0xA2: Reset(4, D); break; //RES 4,D
		case 0xA3: Reset(4, E); break; //RES 4,E
		case 0xA4: Reset(4, H); break; //RES 4,H
		case 0xA5: Reset(4, L); break; //RES 4,L
		case 0xA6: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(4, value); return value;}()); break; //RES 4,(HL)
		case 0xA7: Reset(4, A); break; //RES 4,A
		case 0xA8: Reset(5, B); break; //RES 5,B
		case 0xA9: Reset(5, C); break; //RES 5,C
		case 0xAA: Reset(5, D); break; //RES 5,D
		case 0xAB: Reset(5, E); break; //RES 5,E
		case 0xAC: Reset(5, H); break; //RES 5,H
		case 0xAD: Reset(5, L); break; //RES 5,L
		case 0xAE: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(5, value); return value;}()); break; //RES 5,(HL)
		case 0xAF: Reset(5, A); break; //RES 5,A
		case 0xB0: Reset(6, B); break; //RES 6,B
		case 0xB1: Reset(6, C); break; //RES 6,C
		case 0xB2: Reset(6, D); break; //RES 6,D
		case 0xB3: Reset(6, E); break; //RES 6,E
		case 0xB4: Reset(6, H); break; //RES 6,H
		case 0xB5: Reset(6, L); break; //RES 6,L
		case 0xB6: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(6, value); return value;}()); break; //RES 6,(HL)
		case 0xB7: Reset(6, A); break; //RES 6,A
		case 0xB8: Reset(7, B); break; //RES 7,B
		case 0xB9: Reset(7, C); break; //RES 7,C
		case 0xBA: Reset(7, D); break; //RES 7,D
		case 0xBB: Reset(7, E); break; //RES 7,E
		case 0xBC: Reset(7, H); break; //RES 7,H
		case 0xBD: Reset(7, L); break; //RES 7,L
		case 0xBE: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Reset(7, value); return value;}()); break; //RES 7,(HL)
		case 0xBF: Reset(7, A); break; //RES 7,A

		case 0xC0: Set(0, B); break; //SET 0,B
		case 0xC1: Set(0, C); break; //SET 0,C
		case 0xC2: Set(0, D); break; //SET 0,D
		case 0xC3: Set(0, E); break; //SET 0,E
		case 0xC4: Set(0, H); break; //SET 0,H
		case 0xC5: Set(0, L); break; //SET 0,L
		case 0xC6: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(0, value); return value;}()); break; //SET 0,(HL)
		case 0xC7: Set(0, A); break; //SET 0,A
		case 0xC8: Set(1, B); break; //SET 1,B
		case 0xC9: Set(1, C); break; //SET 1,C
		case 0xCA: Set(1, D); break; //SET 1,D
		case 0xCB: Set(1, E); break; //SET 1,E
		case 0xCC: Set(1, H); break; //SET 1,H
		case 0xCD: Set(1, L); break; //SET 1,L
		case 0xCE: gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(1, value); return value;}()); break; //SET 1,(HL)
		case 0xCF: Set(1, A); break; //SET 1,A
		case 0xD0: { //SET 2,B
			Set(2, B);
		} break;

		case 0xD1: { //SET 2,C
			Set(2, C);
		} break;

		case 0xD2: { //SET 2,D
			Set(2, D);
		} break;

		case 0xD3: { //SET 2,E
			Set(2, E);
		} break;

		case 0xD4: { //SET 2,H
			Set(2, H);
		} break;

		case 0xD5: { //SET 2,L
			Set(2, L);
		} break;

		case 0xD6: { //SET 2,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(2, value); return value;}());
		} break;

		case 0xD7: { //SET 2,A
			Set(2, A);
		} break;

		case 0xD8: { //SET 3,B
			Set(3, B);
		} break;

		case 0xD9: { //SET 3,C
			Set(3, C);
		} break;

		case 0xDA: { //SET 3,D
			Set(3, D);
		} break;

		case 0xDB: { //SET 3,E
			Set(3, E);
		} break;

		case 0xDC: { //SET 3,H
			Set(3, H);
		} break;

		case 0xDD: { //SET 3,L
			Set(3, L);
		} break;

		case 0xDE: { //SET 3,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(3, value); return value;}());
		} break;

		case 0xDF: { //SET 3,A
			Set(3, A);
		} break;

		case 0xE0: { //SET 4,B
			Set(4, B);
		} break;

		case 0xE1: { //SET 4,C
			Set(4, C);
		} break;

		case 0xE2: { //SET 4,D
			Set(4, D);
		} break;

		case 0xE3: { //SET 4,E
			Set(4, E);
		} break;

		case 0xE4: { //SET 4,H
			Set(4, H);
		} break;

		case 0xE5: { //SET 4,L
			Set(4, L);
		} break;

		case 0xE6: { //SET 4,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(4, value); return value;}());
		} break;

		case 0xE7: { //SET 4,A
			Set(4, A);
		} break;

		case 0xE8: { //SET 5,B
			Set(5, B);
		} break;

		case 0xE9: { //SET 5,C
			Set(5, C);
		} break;

		case 0xEA: { //SET 5,D
			Set(5, D);
		} break;

		case 0xEB: { //SET 5,E
			Set(5, E);
		} break;

		case 0xEC: { //SET 5,H
			Set(5, H);
		} break;

		case 0xED: { //SET 5,L
			Set(5, L);
		} break;

		case 0xEE: { //SET 5,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(5, value); return value;}());
		} break;

		case 0xEF: { //SET 5,A
			Set(5, A);
		} break;

		case 0xF0: { //SET 6,B
			Set(6, B);
		} break;

		case 0xF1: { //SET 6,C
			Set(6, C);
		} break;

		case 0xF2: { //SET 6,D
			Set(6, D);
		} break;

		case 0xF3: { //SET 6,E
			Set(6, E);
		} break;

		case 0xF4: { //SET 6,H
			Set(6, H);
		} break;

		case 0xF5: { //SET 6,L
			Set(6, L);
		} break;

		case 0xF6: { //SET 6,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(6, value); return value;}());
		} break;

		case 0xF7: { //SET 6,A
			Set(6, A);
		} break;

		case 0xF8: { //SET 7,B
			Set(7, B);
		} break;

		case 0xF9: { //SET 7,C
			Set(7, C);
		} break;

		case 0xFA: { //SET 7,D
			Set(7, D);
		} break;

		case 0xFB: { //SET 7,E
			Set(7, E);
		} break;

		case 0xFC: { //SET 7,H
			Set(7, H);
		} break;

		case 0xFD: { //SET 7,L
			Set(7, L);
		} break;

		case 0xFE: { //SET 7,(HL)
			gb.mem.Write(HL, [&](){u8 value = GetLEBytes<u8>(HL); Set(7, value); return value;}());
		} break;

		case 0xFF: { //SET 7,A
			Set(7, A);
		} break;

		default: fmt::printf("Unimplemented CB: 0x%02X\n", opcode); break;
	}
}

u8 CPU::FetchOpcode(u16 counter) {
	if(counter > 0xFFFF) throw "[CPU::FetchOpcode] counter is bigger than 0xFFFF";  //this should never happen

	return GetLEBytes<u8>(counter);
}

bool CPU::CheckZero() {
	return (F & Zero_Set);
}

bool CPU::CheckNegative() {
	return (F & Negative_Set);
}

bool CPU::CheckHalfCarry() {
	return (F & HalfCarry_Set);
}

bool CPU::CheckCarry() {
	return (F & Carry_Set);
}


u16 CPU::setFlags(u16 flags, u16 ans, u8 old, u8 diff) {
	F &= ~(((flags & 0xF00) >> 4) | (flags & 0xF0) | (flags << 4)); //reset flags and takes care of "unsets"

	if(flags & Zero)				F |= (ans & 0xff) ? 0 : Zero_Set;
	else if((flags & Zero_Set))		F |= Zero_Set;
	if(flags & Negative_Set)		F |= Negative_Set;
	if(flags & HalfCarry)			F |= (((old ^ diff ^ ans) & 0x10) == 0x10 ? HalfCarry_Set : 0);
	else if(flags & HalfCarry_Set)	F |= HalfCarry_Set;
	if(flags & Carry)				F |= ((ans > 0xff) ? Carry_Set : 0);
	else if(flags & Carry_Set)		F |= Carry_Set;

	return ans;
}

template <typename T>
T CPU::GetLEBytes() {
	return GetLEBytes<T>(PC, true);
}

template <typename T>
T CPU::GetLEBytes(u16 addr) {
	return GetLEBytes<T>(addr, false);
}

template <typename T>
T CPU::GetLEBytes(u16& addr, bool increase) {
	T ret = 0, amount = sizeof(T);
	while(amount > 0) ret += u8(gb.mem.Read((increase) ? addr++ : addr + (sizeof(T) - amount))) << ((sizeof(T) - amount--) * 8);
	return ret;
}

void CPU::Add(u16 in) {
	//u16 temp = setFlags(Negative_Unset | HalfCarry | Carry, L + (in & 0xff), L, (in & 0xff));
	u16 temp = L + (in & 0xff); L = u8(temp);
	H = u8(setFlags(Negative_Unset | HalfCarry | Carry, H + (temp >> 8) + (in >> 8), H, (in >> 8)));
}

void CPU::Add(u8 in, bool carry) {
	A = u8(setFlags(Negative_Unset | Zero | HalfCarry | Carry, A + in + carry, A, in));
}

void CPU::Sub(u8 in, bool carry) {
	A = u8(setFlags(Negative_Set | Zero | HalfCarry | Carry, A - in - carry, A, in));
}

void CPU::And(u8 in) {
	A = u8(setFlags(Negative_Unset | HalfCarry_Set | Carry_Unset | Zero, A & in));
}

void CPU::Xor(u8 in) {
	A = u8(setFlags(Negative_Unset | HalfCarry_Unset | Carry_Unset | Zero, A ^ in));
}

void CPU::Or(u8 in) {
	A = u8(setFlags(Negative_Unset | HalfCarry_Unset | Carry_Unset | Zero, A | in));
}

void CPU::Compare(u8 in) {
	u8(setFlags(Negative_Set | Zero | HalfCarry | Carry, A - in, A, in));
}

void CPU::Increase(u8& reg) {
	reg = u8(setFlags(Negative_Unset | Zero | HalfCarry, reg + 1, reg, 1));
}

void CPU::Decrease(u8& reg) {
	reg = u8(setFlags(Negative_Set | Zero | HalfCarry, reg - 1, reg, 1));
}

void CPU::Increase(u16& reg) {
	reg++;
}

void CPU::Decrease(u16& reg) {
	reg--;
}

void CPU::RotateLeft(u8& reg, bool carry) {
	u8 bit0 = (reg & 0x80) >> 7;
	reg = u8(setFlags((bit0 ? Carry_Set : Carry_Unset) | Negative_Unset | HalfCarry_Unset | Zero, (carry ? CheckCarry() : bit0) | (reg << 1)));
}

void CPU::RotateRight(u8& reg, bool carry) {
	u8 bit0 = (reg & 0x1);
	reg = u8(setFlags((bit0 ? Carry_Set : Carry_Unset) | Negative_Unset | HalfCarry_Unset | Zero, ((carry ? CheckCarry() : bit0) << 7) | (reg >> 1)));
}

void CPU::ShiftLeftArithmetic(u8& reg) {
	reg = u8(setFlags(((reg & 0x80) ? Carry_Set : Carry_Unset) | Negative_Unset | HalfCarry_Unset | Zero, reg << 1));
}

void CPU::ShiftRightArithmetic(u8& reg) {
	reg = u8(setFlags(((reg & 0x1) ? Carry_Set : Carry_Unset) | Negative_Unset | HalfCarry_Unset | Zero, (reg & 0x80) | (reg >> 1)));
}

void CPU::Swap(u8& reg) {
	reg = u8(setFlags(Negative_Unset | HalfCarry_Unset | Carry_Unset | Zero, (reg << 4) | (reg >> 4)));
}

void CPU::ShiftRightLogical(u8& reg) {
	reg = u8(setFlags(((reg & 0x1) ? Carry_Set : Carry_Unset) | Negative_Unset | HalfCarry_Unset | Zero, reg >> 1));
}

void CPU::Bit(u8 bit, u8 reg) {
	setFlags(Negative_Unset | HalfCarry_Set | Zero, reg & (1 << bit));
}

void CPU::Set(u8 bit, u8& reg) {
	reg |= (1 << bit);
}

void CPU::Reset(u8 bit, u8& reg) {
	reg &= ~(1 << bit);
}

void CPU::Load(u8& loc, u8 val) {
	loc = val;
}

void CPU::Load(u16& loc, u16 val) {
	loc = val;
}

void CPU::Push(u16& reg_pair) {
	SP -= 2;
	gb.mem.Write(SP, reg_pair);
}

void CPU::Pop(u16& reg_pair) {
	reg_pair = GetLEBytes<u16>(SP);
	SP += 2;
}

void CPU::Jump(u16 loc, bool cond) {
	if(cond) PC = loc;
}

void CPU::JumpRelative(s8 offset, bool cond) {
	if(cond) PC += offset;
}

void CPU::Call(u16 loc, bool cond) {
	if(cond) {
		Push(PC);
		Jump(loc);
	}
}

void CPU::Ret() {
	Pop(PC);
}

void CPU::Rst(u8 loc) {
	Call(loc);
}