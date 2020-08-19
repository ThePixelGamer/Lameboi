#include "CPU.h"

#include "Gameboy.h"

#include <stdexcept>
#include <array>
#include "fmt/format.h"
#include "fmt/printf.h"

const std::array<Instruction, 0x100> instTable {{
//             0                              1                   2								3                 4                         5                       6			         7                     8                        9               A                     B                C                      D                       E                F
	{"NOP",						0}, {"LD BC, {:04x}",	3}, {"LD (BC), A",				0}, {"INC BC",		0}, {"INC B",			0}, {"DEC B",		0}, {"LD B, {:02x}",    3}, {"RLCA",		0}, {"LD ({:04x}), SP",	3}, {"ADD HL, BC", 0}, {"LD A, (BC)",     0}, {"DEC BC",  0}, {"INC C",			 0}, {"DEC C",		 0}, {"LD C, {:02x}", 3}, {"RRCA",    0}, //00
	{"STOP",					0}, {"LD DE, {:04x}",	3}, {"LD (DE), A",				0}, {"INC DE",		0}, {"INC D",			0}, {"DEC D",		0}, {"LD D, {:02x}",    3}, {"RLA",		0}, {"JR {:02x}",		3}, {"ADD HL, DE", 0}, {"LD A, (DE)",     0}, {"DEC DE",  0}, {"INC E",			 0}, {"DEC E",		 0}, {"LD E, {:02x}", 3}, {"RRA",     0}, //10
	{"JR NZ, {:02x}",			3}, {"LD HL, {:04x}",	3}, {"LD (HL+), A",				0}, {"INC HL",		0}, {"INC H",			0}, {"DEC H",		0}, {"LD H, {:02x}",    3}, {"DAA",		0}, {"JR Z, {:02x}",	3}, {"ADD HL, HL", 0}, {"LD A, (HL+)",    0}, {"DEC HL",  0}, {"INC L",			 0}, {"DEC L",		 0}, {"LD L, {:02x}", 3}, {"CPL",     0}, //20
	{"JR NC, {:02x}",			3}, {"LD SP, {:04x}",	3}, {"LD (HL-), A",				0}, {"INC SP",		0}, {"INC (HL)",		0}, {"DEC (HL)",	0}, {"LD (HL), {:02x}", 3}, {"SCF",		0}, {"JR C, {:02x}",	3}, {"ADD HL, SP", 0}, {"LD A, (HL-)",    0}, {"DEC SP",  0}, {"INC A",			 0}, {"DEC A",		 0}, {"LD A, {:02x}", 3}, {"CCF",     0}, //30
	{"LD B, B",					0}, {"LD B, C",			0}, {"LD B, D",					0}, {"LD B, E",		0}, {"LD B, H",			0}, {"LD B, L",		0}, {"LD B, (HL)",		 0}, {"LD B, A",    0}, {"LD C, B",         0}, {"LD C, C",    0}, {"LD C, D",        0}, {"LD C, E", 0}, {"LD C, H",		 0}, {"LD C, L",	 0}, {"LD C, (HL)",    0}, {"LD C, A", 0}, //40
	{"LD D, B",					0}, {"LD D, C",			0}, {"LD D, D",					0}, {"LD D, E",		0}, {"LD D, H",			0}, {"LD D, L",		0}, {"LD D, (HL)",		 0}, {"LD D, A",    0}, {"LD E, B",         0}, {"LD E, C",    0}, {"LD E, D",        0}, {"LD E, E", 0}, {"LD E, H",		 0}, {"LD E, L",	 0}, {"LD E, (HL)",    0}, {"LD E, A", 0}, //50
	{"LD H, B",					0}, {"LD H, C",			0}, {"LD H, D",					0}, {"LD H, E",		0}, {"LD H, H",			0}, {"LD H, L",		0}, {"LD H, (HL)",		 0}, {"LD H, A",    0}, {"LD L, B",         0}, {"LD L, C",    0}, {"LD L, D",        0}, {"LD L, E", 0}, {"LD L, H",		 0}, {"LD L, L",	 0}, {"LD L, (HL)",    0}, {"LD L, A", 0}, //60
	{"LD (HL), B",				0}, {"LD (HL), C",		0}, {"LD (HL), D",				0}, {"LD (HL), E",	0}, {"LD (HL), H",		0}, {"LD (HL), L",	0}, {"HALT",			 0}, {"LD (HL), A", 0}, {"LD A, B",         0}, {"LD A, C",    0}, {"LD A, D",        0}, {"LD A, E", 0}, {"LD A, H",		 0}, {"LD A, L",	 0}, {"LD A, (HL)",    0}, {"LD A, A", 0}, //70
	{"ADD B",					0}, {"ADD C",			0}, {"ADD D",					0}, {"ADD E",		0}, {"ADD H",			0}, {"ADD L",		0}, {"ADD (HL)",		 0}, {"ADD A",		0}, {"ADC B",			0}, {"ADC C",	   0}, {"ADC D",		  0}, {"ADC E",   0}, {"ADC H",			 0}, {"ADC L",		 0}, {"ADC (HL)",	   0}, {"ADC A",   0}, //80
	{"SUB B",					0}, {"SUB C",			0}, {"SUB D",					0}, {"SUB E",		0}, {"SUB H",			0}, {"SUB L",		0}, {"SUB (HL)",		 0}, {"SUB A",		0}, {"SBC B",			0}, {"SBC C",	   0}, {"SBC D",		  0}, {"SBC E",   0}, {"SBC H",			 0}, {"SBC L",		 0}, {"SBC (HL)",	   0}, {"SBC A",   0}, //90
	{"AND B",					0}, {"AND C",			0}, {"AND D",					0}, {"AND E",		0}, {"AND H",			0}, {"AND L",		0}, {"AND (HL)",		 0}, {"AND A",		0}, {"XOR B",			0}, {"XOR C",	   0}, {"XOR D",		  0}, {"XOR E",   0}, {"XOR H",			 0}, {"XOR L",		 0}, {"XOR (HL)",	   0}, {"XOR A",   0}, //a0
	{"OR B",					0}, {"OR C",			0}, {"OR D",					0}, {"OR E",		0}, {"OR H",			0}, {"OR L",		0}, {"OR (HL)",			 0}, {"OR A",		0}, {"CP B",			0}, {"CP C",	   0}, {"CP D",			  0}, {"CP E",    0}, {"CP H",			 0}, {"CP L",		 0}, {"CP (HL)",	   0}, {"CP A",    0}, //b0
	{"RET NZ",					0}, {"POP BC",			0}, {"JP NZ, {:04x}",			3}, {"JP {:04x}",	3}, {"CALL NZ, {:04x}",	3}, {"PUSH BC",		0}, {"ADD A, {:02x}",	 3}, {"RST $00",	0}, {"RET Z",			0}, { "RET",       0}, {"JP Z, $hX",	  3}, {"CB:",     0}, {"CALL Z, {:04x}", 3}, {"CALL {:04x}", 3}, {"ADC {:02x}",   3}, {"RST $08", 0}, //c0
	{"RET NC",					0}, {"POP DE",			0}, {"JP NC, {:04x}",			3}, {"",			0}, {"CALL NC, {:04x}",	3}, {"PUSH DE",		0}, {"SUB {:02x}",		 3}, {"RST $10",	0}, {"RET C",			0}, { "RETI",      0}, {"JP C, $hX",	  3}, {"",        0}, {"CALL C, {:04x}", 3}, { "",			 0}, {"SBC {:02x}",   3}, {"RST $18", 0}, //d0
	{"LD ({:04x}+{:02x}), A",	2}, {"POP HL",			0}, {"LD ({:04x}+{:02x}), A",	1}, {"",			0}, {"",				0}, {"PUSH HL",		0}, {"AND {:02x}",		 3}, {"RST $20",	0}, {"ADD SP,%hhd",		3}, { "JP HL",     0}, {"LD ($%02hX), A", 3}, {"",        0}, {"",				 0}, { "",			 0}, {"XOR {:02x}",   3}, {"RST $28", 0}, //e0
	{"LD A, ({:04x}+{:02x})",	2}, {"POP AF",			0}, {"LD A, ({:04x}+{:02x})",	1}, {"DI",			0}, {"",				0}, {"PUSH AF",		0}, {"OR {:02x}",		 3}, {"RST $30",	0}, {"LD HL, SP+%hhd",	3}, {"LD SP,HL",   0}, {"LD A, ($%02hX)", 3}, {"EI",      0}, {"",				 0}, { "",			 0}, {"CP {:02x}",    3}, {"RST $38", 0}, //f0
}};
const std::array<Instruction, 0x100> CBTable{{
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
	/*
	gb.log << fmt::format("{:04x} ", PC-1); //PSI

	Instruction inst = (opcode == 0xCB) ? CBTable[FetchOpcode(PC)] : instTable[opcode];
	auto qwqs = std::string(inst.name);
	auto qwq = qwqs.c_str();
	switch(inst.arg) {
		case 0: gb.log << qwq; break;
		case 1: gb.log << fmt::format(qwq, 0xFF00, C); break;
		case 2: gb.log << fmt::format(qwq, 0xFF00, GetLEBytes<u8>(PC)); break;
		case 3: gb.log << fmt::format(qwq, GetLEBytes<u16>(PC)); break;
	}
	*/

	/*
	if (PC >= 0x100) {
		gb.log << fmt::format("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X})", A, (AF & 0xFF), B, C, D, E, H, L, SP, PC, gb.mem.Read(PC), gb.mem.Read(PC + 1), gb.mem.Read(PC + 2), gb.mem.Read(PC + 3));
		gb.log << "\n";
	}
	*/

	//fprintf(gb.log, "PC:%04x OPC:%02x %02x %02x C:%x H:%x N:%x Z:%x A:%02x\n", PC - 1, opcode, GetLEBytes<u8>(PC), GetLEBytes<u8>(PC + 1), CheckCarry(), CheckHalfCarry(), CheckNegative(), CheckZero(), A); //LilaQ
}

CPU::CPU(Gameboy& t_gb) : gb(t_gb) {
	clean();
}

void CPU::clean() {
	AF = 0;
	BC = 0;
	DE = 0;
	HL = 0;
	SP = 0;
	PC = 0;
	opcode = 0;
	gb.IME = false;

	haltBug = false;
	lowPower = false;
	handler = false;
}

template<typename T, typename... Args>
u8 CPU::M_Write_Helper(T func, Args... args) {
	u8 value = GetLEBytes<u8>(HL);
	(this->*func)(value, args...);
	return value;
}

void CPU::interrupt(u8 interrupt) {
	//2 nops
	gb.scheduler.newMCycle();
	gb.scheduler.newMCycle();

	Push(PC);

	PC = interrupt;
	gb.scheduler.newMCycle();
}

bool CPU::interruptPending() {
	return gb.mem.Read(0xFF0F) != 0xE0;
}

bool CPU::handleInterrupts() {
	if (gb.IME && interruptPending()) {
		if (gb.mem.IE.vblank && gb.mem.IF.vblank) {
			gb.mem.IF.vblank = 0;

			interrupt(0x40);
		}

		if (gb.mem.IE.lcdStat && gb.mem.IF.lcdStat) {
			gb.mem.IF.lcdStat = 0;

			interrupt(0x48);
		}

		if (gb.mem.IE.timer && gb.mem.IF.timer) {
			gb.mem.IF.timer = 0;

			interrupt(0x50);
		}

		if (gb.mem.IE.joypad && gb.mem.IF.joypad) {
			gb.mem.IF.joypad = 0;

			interrupt(0x60);
		}

		return true;
	}

	return false;
}

void CPU::ExecuteOpcode() {
	if (lowPower) {
		if (handler && handleInterrupts()) {
			handler = false;
			lowPower = false;
		}
		else if (interruptPending()) {
			lowPower = false;
		}
		else {
			gb.scheduler.newMCycle(); //not sure about how I'm handling it
			return;
		}
	}
	else {
		handleInterrupts();
	}

	handlePrint();

	if (haltBug) {
		opcode = FetchOpcode(PC);
		haltBug = false;
	}
	else {
		opcode = FetchOpcode(PC++);
	}

	using overloaded = void (CPU::*)(u8&);

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
		case 0x34: write(HL, M_Write_Helper(static_cast<overloaded>(&CPU::Increase))); break; //INC (HL)
		case 0x3C: Increase(A); break;

		case 0x05: Decrease(B); break; //DEC B
		case 0x0D: Decrease(C); break; //DEC C
		case 0x15: Decrease(D); break; //DEC D
		case 0x1D: Decrease(E); break; //DEC E
		case 0x25: Decrease(H); break; //DEC H
		case 0x2D: Decrease(L); break; //DEC L
		case 0x35: write(HL, M_Write_Helper(static_cast<overloaded>(&CPU::Decrease))); break; //DEC (HL)		
		case 0x3D: Decrease(A); break;
			
		case 0x01: Load(BC, GetLEBytes<u16>()); break; //LD BC,u16
		case 0x11: Load(DE, GetLEBytes<u16>()); break; //LD DE,u16
		case 0x21: Load(HL, GetLEBytes<u16>()); break; //LD HL,u16
		case 0x31: Load(SP, GetLEBytes<u16>()); break; //LD SP,u16
		case 0x08: write(GetLEBytes<u16>(), SP); break; //LD (u16),SP
			
		case 0x02: write(BC, A); break; //LD (BC),A
		case 0x12: write(DE, A); break; //LD (DE),A
		case 0x22: write(HL++, A); break; //LD (HL+),A
		case 0x32: write(HL--, A); break; //LD (HL-),A

		case 0x06: Load(B, GetLEBytes<u8>()); break; //LD B,u8
		case 0x0E: Load(C, GetLEBytes<u8>()); break; //LD C,u8
		case 0x16: Load(D, GetLEBytes<u8>()); break; //LD D,u8
		case 0x1E: Load(E, GetLEBytes<u8>()); break; //LD E,u8
		case 0x26: Load(H, GetLEBytes<u8>()); break; //LD H,u8
		case 0x2E: Load(L, GetLEBytes<u8>()); break; //LD L,u8
		case 0x36: write(HL, GetLEBytes<u8>()); break; //LD (HL),u8
		case 0x3E: Load(A, GetLEBytes<u8>()); break; //LD A,u8

		case 0x0A: Load(A, GetLEBytes<u8>(BC)); break; //LD A,(BC)
		case 0x1A: Load(A, GetLEBytes<u8>(DE)); break; //LD A,(DE)
		case 0x2A: Load(A, GetLEBytes<u8>(HL++)); break; //LD A,(HL+)
		case 0x3A: Load(A, GetLEBytes<u8>(HL--)); break; //LD A,(HL-)
			
		case 0x09: Add(BC); break; //ADD HL,BC
		case 0x19: Add(DE); break; //ADD HL,DE
		case 0x29: Add(HL); break; //ADD HL,HL
		case 0x39: Add(SP); break; //ADD HL,SP
			
		case 0x07: RotateLeft(A); SetZero(false); break; //RLCA
		case 0x0F: RotateRight(A); SetZero(false); break;  //RRCA
		case 0x17: RotateLeft(A, true); SetZero(false); break; //RLA
		case 0x1F: RotateRight(A, true); SetZero(false); break; //RRA

		case 0x18: JumpRelative(GetLEBytes<u8>()); break; //JR i8
		case 0x20: JumpRelative(GetLEBytes<u8>(), !CheckZero()); break; //JR NZ,i8
		case 0x30: JumpRelative(GetLEBytes<u8>(), !CheckCarry()); break; //JR NC,i8
		case 0x28: JumpRelative(GetLEBytes<u8>(), CheckZero()); break; //JR Z,i8
		case 0x38: JumpRelative(GetLEBytes<u8>(), CheckCarry()); break; //JR C,i8

		case 0x00: break; //NOP
		//case 0x10: /*throw "Stop called";*/ break; //STOP

		case 0x27: { //DAA
			// note: assumes a is a uint8_t and wraps from 0xff to 0
			if (!CheckNegative()) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
				if (CheckCarry() || A > 0x99) { 
					A += 0x60; 
					SetCarry(true); 
				}
				if (CheckHalfCarry() || (A & 0x0f) > 0x09) { 
					A += 0x6; 
				}
			} 
			else {  // after a subtraction, only adjust if (half-)carry occurred
				if (CheckCarry()) { 
					A -= 0x60; 
				}
				if (CheckHalfCarry()) { 
					A -= 0x6; 
				}
			}
			// these flags are always updated
			SetZero(A);
			SetHalfCarry(false);
		} break;

		case 0x2F: { //CPL
			A = ~A;
			SetNegative(true);
			SetHalfCarry(true);
		} break;

		case 0x37: { //SCF
			SetCarry(true);
			SetNegative(false);
			SetHalfCarry(false);
		} break;
			
		case 0x3F: { //CCF
			SetCarry(!CheckCarry());
			SetNegative(false);
			SetHalfCarry(false);
		} break;

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
		case 0x70: write(HL, B); break;
		case 0x71: write(HL, C); break;
		case 0x72: write(HL, D); break;
		case 0x73: write(HL, E); break;
		case 0x74: write(HL, H); break;
		case 0x75: write(HL, L); break;
		
		case 0x76: 
		{
			if (gb.IME) {
				handler = true;
				lowPower = true;
			}
			else if (gb.mem.Interrupt & gb.mem.Read(0xFF0F) & 0x1F) {
				haltBug = true;
			}
			else {
				lowPower = true;
			}
		} break;

		case 0x77: write(HL, A); break;
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
		case 0xF1: Pop(AF); AF &= 0xFFF0; break; //POP AF
			
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
			SetFlags(HalfCarry | Carry, (SP & 0xff) + offset, u8(SP), offset);
			SetNegative(false);
			SetZero(false);
			SP += s8(offset);
		} break;

		case 0xF8: { //LD HL,SP+i8
			u8 offset = GetLEBytes<u8>();
			SetFlags(HalfCarry | Carry, (SP & 0xff) + offset, u8(SP), offset);
			SetNegative(false);
			SetZero(false);
			Load(HL, SP+s8(offset));
		} break;

		case 0xF9: Load(SP, HL); break; //LD SP,HL			
		case 0xF0: Load(A, GetLEBytes<u8>(GetLEBytes<u8>() + 0xFF00)); break; //LD A,(FF00+u8)
		case 0xF2: Load(A, GetLEBytes<u8>(0xFF00 + C)); break; //LD A,(FF00+C)
		case 0xFA: Load(A, GetLEBytes<u8>(GetLEBytes<u16>())); break; //LD A,(u16)
		case 0xE0: write(0xFF00 + GetLEBytes<u8>(), A); break; //LD (FF00+u8),A
		case 0xE2: write(0xFF00 + C, A); break; //LD (FF00+C),A
		case 0xEA: write(GetLEBytes<u16>(), A); break; //LD (u16),A

		case 0xF3: gb.IME = false; break; //DI
		case 0xFB: gb.IME = true; break; //EI
		case 0xCB: handleCB(); break;

		default: fmt::printf("Unimplemented Opcode: 0x%02X\n", opcode); break;
	}

    //fprintf(gb.log, " AF: $%04X BC: $%04X DE: $%04X HL: $%04X\n", AF, BC, DE, HL);
}

void CPU::handleCB() {
	opcode = FetchOpcode(PC++);

	switch (opcode) {
		case 0x00: RotateLeft(B); break; //RLC B
		case 0x01: RotateLeft(C); break; //RLC C
		case 0x02: RotateLeft(D); break; //RLC D
		case 0x03: RotateLeft(E); break; //RLC E
		case 0x04: RotateLeft(H); break; //RLC H
		case 0x05: RotateLeft(L); break; //RLC L
		case 0x06: write(HL, M_Write_Helper(&CPU::RotateLeft, false)); break; //RLC (HL)
		case 0x07: RotateLeft(A); break; //RLC A
		case 0x10: RotateLeft(B, true); break; //RL B
		case 0x11: RotateLeft(C, true); break; //RL C
		case 0x12: RotateLeft(D, true); break; //RL D
		case 0x13: RotateLeft(E, true); break; //RL E
		case 0x14: RotateLeft(H, true); break; //RL H
		case 0x15: RotateLeft(L, true); break; //RL L
		case 0x16: write(HL, M_Write_Helper(&CPU::RotateLeft, true)); break; //RL (HL)
		case 0x17: RotateLeft(A, true); break; //RL A

		case 0x08: RotateRight(B); break; //RRC B
		case 0x09: RotateRight(C); break; //RRC C
		case 0x0A: RotateRight(D); break; //RRC D
		case 0x0B: RotateRight(E); break; //RRC E
		case 0x0C: RotateRight(H); break; //RRC H
		case 0x0D: RotateRight(L); break; //RRC L
		case 0x0E: write(HL, M_Write_Helper(&CPU::RotateRight, false)); break; //RRC (HL)
		case 0x0F: RotateRight(A); break; //RRC A
		case 0x18: RotateRight(B, true); break; //RR B
		case 0x19: RotateRight(C, true); break; //RR C
		case 0x1A: RotateRight(D, true); break; //RR D
		case 0x1B: RotateRight(E, true); break; //RR E
		case 0x1C: RotateRight(H, true); break; //RR H
		case 0x1D: RotateRight(L, true); break; //RR L
		case 0x1E: write(HL, M_Write_Helper(&CPU::RotateRight, true)); break; //RR (HL)
		case 0x1F: RotateRight(A, true); break; //RR A

		case 0x20: ShiftLeftArithmetic(B); break; //SLA B
		case 0x21: ShiftLeftArithmetic(C); break; //SLA C
		case 0x22: ShiftLeftArithmetic(D); break; //SLA D
		case 0x23: ShiftLeftArithmetic(E); break; //SLA E
		case 0x24: ShiftLeftArithmetic(H); break; //SLA H
		case 0x25: ShiftLeftArithmetic(L); break; //SLA L
		case 0x26: write(HL, M_Write_Helper(&CPU::ShiftLeftArithmetic)); break; //SLA (HL)
		case 0x27: ShiftLeftArithmetic(A); break; //SLA A

		case 0x28: ShiftRightArithmetic(B); break; //SRA B
		case 0x29: ShiftRightArithmetic(C); break; //SRA C
		case 0x2A: ShiftRightArithmetic(D); break; //SRA D
		case 0x2B: ShiftRightArithmetic(E); break; //SRA E
		case 0x2C: ShiftRightArithmetic(H); break; //SRA H
		case 0x2D: ShiftRightArithmetic(L); break; //SRA L
		case 0x2E: write(HL, M_Write_Helper(&CPU::ShiftRightArithmetic)); break; //SRA (HL)
		case 0x2F: ShiftRightArithmetic(A); break; //SRA A

		case 0x30: Swap(B); break; //SWAP B
		case 0x31: Swap(C); break; //SWAP C
		case 0x32: Swap(D); break; //SWAP D
		case 0x33: Swap(E); break; //SWAP E
		case 0x34: Swap(H); break; //SWAP H
		case 0x35: Swap(L); break; //SWAP L
		case 0x36: write(HL, M_Write_Helper(&CPU::Swap)); break; //SWAP (HL)
		case 0x37: Swap(A); break; //SWAP A

		case 0x38: ShiftRightLogical(B); break; //SRL B
		case 0x39: ShiftRightLogical(C); break; //SRL C
		case 0x3A: ShiftRightLogical(D); break; //SRL D
		case 0x3B: ShiftRightLogical(E); break; //SRL E
		case 0x3C: ShiftRightLogical(H); break; //SRL H
		case 0x3D: ShiftRightLogical(L); break; //SRL L
		case 0x3E: write(HL, M_Write_Helper(&CPU::ShiftRightLogical)); break; //SRL (HL)
		case 0x3F: ShiftRightLogical(A); break; //SRL A

		case 0x40: Bit(B, 0); break; //BIT 0,B
		case 0x41: Bit(C, 0); break; //BIT 0,C
		case 0x42: Bit(D, 0); break; //BIT 0,D
		case 0x43: Bit(E, 0); break; //BIT 0,E
		case 0x44: Bit(H, 0); break; //BIT 0,H
		case 0x45: Bit(L, 0); break; //BIT 0,L
		case 0x46: Bit(GetLEBytes<u8>(HL), 0); break; //BIT 0,(HL)
		case 0x47: Bit(A, 0); break; //BIT 0,A
		case 0x48: Bit(B, 1); break; //BIT 1,B
		case 0x49: Bit(C, 1); break; //BIT 1,C
		case 0x4A: Bit(D, 1); break; //BIT 1,D
		case 0x4B: Bit(E, 1); break; //BIT 1,E
		case 0x4C: Bit(H, 1); break; //BIT 1,H
		case 0x4D: Bit(L, 1); break; //BIT 1,L
		case 0x4E: Bit(GetLEBytes<u8>(HL), 1); break; //BIT 1,(HL)
		case 0x4F: Bit(A, 1); break; //BIT 1,A
		case 0x50: Bit(B, 2); break; //BIT 2,B
		case 0x51: Bit(C, 2); break; //BIT 2,C
		case 0x52: Bit(D, 2); break; //BIT 2,D
		case 0x53: Bit(E, 2); break; //BIT 2,E
		case 0x54: Bit(H, 2); break; //BIT 2,H
		case 0x55: Bit(L, 2); break; //BIT 2,L
		case 0x56: Bit(GetLEBytes<u8>(HL), 2); break; //BIT 2,(HL)
		case 0x57: Bit(A, 2); break; //BIT 2,A
		case 0x58: Bit(B, 3); break; //BIT 3,B
		case 0x59: Bit(C, 3); break; //BIT 3,C
		case 0x5A: Bit(D, 3); break; //BIT 3,D
		case 0x5B: Bit(E, 3); break; //BIT 3,E
		case 0x5C: Bit(H, 3); break; //BIT 3,H
		case 0x5D: Bit(L, 3); break; //BIT 3,L
		case 0x5E: Bit(GetLEBytes<u8>(HL), 3); break; //BIT 3,(HL)
		case 0x5F: Bit(A, 3); break; //BIT 3,A
		case 0x60: Bit(B, 4); break; //BIT 4,B
		case 0x61: Bit(C, 4); break; //BIT 4,C
		case 0x62: Bit(D, 4); break; //BIT 4,D
		case 0x63: Bit(E, 4); break; //BIT 4,E
		case 0x64: Bit(H, 4); break; //BIT 4,H
		case 0x65: Bit(L, 4); break; //BIT 4,L
		case 0x66: Bit(GetLEBytes<u8>(HL), 4); break; //BIT 4,(HL)
		case 0x67: Bit(A, 4); break; //BIT 4,A
		case 0x68: Bit(B, 5); break; //BIT 5,B
		case 0x69: Bit(C, 5); break; //BIT 5,C
		case 0x6A: Bit(D, 5); break; //BIT 5,D
		case 0x6B: Bit(E, 5); break; //BIT 5,E
		case 0x6C: Bit(H, 5); break; //BIT 5,H
		case 0x6D: Bit(L, 5); break; //BIT 5,L
		case 0x6E: Bit(GetLEBytes<u8>(HL), 5); break; //BIT 5,(HL)
		case 0x6F: Bit(A, 5); break; //BIT 5,A
		case 0x70: Bit(B, 6); break; //BIT 6,B
		case 0x71: Bit(C, 6); break; //BIT 6,C
		case 0x72: Bit(D, 6); break; //BIT 6,D
		case 0x73: Bit(E, 6); break; //BIT 6,E
		case 0x74: Bit(H, 6); break; //BIT 6,H
		case 0x75: Bit(L, 6); break; //BIT 6,L
		case 0x76: Bit(GetLEBytes<u8>(HL), 6); break; //BIT 6,(HL)
		case 0x77: Bit(A, 6); break; //BIT 6,A
		case 0x78: Bit(B, 7); break; //BIT 7,B
		case 0x79: Bit(C, 7); break; //BIT 7,C
		case 0x7A: Bit(D, 7); break; //BIT 7,D
		case 0x7B: Bit(E, 7); break; //BIT 7,E
		case 0x7C: Bit(H, 7); break; //BIT 7,H
		case 0x7D: Bit(L, 7); break; //BIT 7,L
		case 0x7E: Bit(GetLEBytes<u8>(HL), 7); break; //BIT 7,(HL)
		case 0x7F: Bit(A, 7); break; //BIT 7,A

		case 0x80: Reset(B, 0); break; //RES 0,B
		case 0x81: Reset(C, 0); break; //RES 0,C
		case 0x82: Reset(D, 0); break; //RES 0,D
		case 0x83: Reset(E, 0); break; //RES 0,E
		case 0x84: Reset(H, 0); break; //RES 0,H
		case 0x85: Reset(L, 0); break; //RES 0,L
		case 0x86: write(HL, M_Write_Helper(&CPU::Reset, 0)); break; //RES 0,(HL)
		case 0x87: Reset(A, 0); break; //RES 0,A
		case 0x88: Reset(B, 1); break; //RES 1,B
		case 0x89: Reset(C, 1); break; //RES 1,C
		case 0x8A: Reset(D, 1); break; //RES 1,D
		case 0x8B: Reset(E, 1); break; //RES 1,E
		case 0x8C: Reset(H, 1); break; //RES 1,H
		case 0x8D: Reset(L, 1); break; //RES 1,L
		case 0x8E: write(HL, M_Write_Helper(&CPU::Reset, 1)); break; //RES 1,(HL)
		case 0x8F: Reset(A, 1); break; //RES 1,A
		case 0x90: Reset(B, 2); break; //RES 2,B
		case 0x91: Reset(C, 2); break; //RES 2,C
		case 0x92: Reset(D, 2); break; //RES 2,D
		case 0x93: Reset(E, 2); break; //RES 2,E
		case 0x94: Reset(H, 2); break; //RES 2,H
		case 0x95: Reset(L, 2); break; //RES 2,L
		case 0x96: write(HL, M_Write_Helper(&CPU::Reset, 2)); break; //RES 2,(HL)
		case 0x97: Reset(A, 2); break; //RES 2,A
		case 0x98: Reset(B, 3); break; //RES 3,B
		case 0x99: Reset(C, 3); break; //RES 3,C
		case 0x9A: Reset(D, 3); break; //RES 3,D
		case 0x9B: Reset(E, 3); break; //RES 3,E
		case 0x9C: Reset(H, 3); break; //RES 3,H
		case 0x9D: Reset(L, 3); break; //RES 3,L
		case 0x9E: write(HL, M_Write_Helper(&CPU::Reset, 3)); break; //RES 3,(HL)
		case 0x9F: Reset(A, 3); break; //RES 3,A
		case 0xA0: Reset(B, 4); break; //RES 4,B
		case 0xA1: Reset(C, 4); break; //RES 4,C
		case 0xA2: Reset(D, 4); break; //RES 4,D
		case 0xA3: Reset(E, 4); break; //RES 4,E
		case 0xA4: Reset(H, 4); break; //RES 4,H
		case 0xA5: Reset(L, 4); break; //RES 4,L
		case 0xA6: write(HL, M_Write_Helper(&CPU::Reset, 4)); break; //RES 4,(HL)
		case 0xA7: Reset(A, 4); break; //RES 4,A
		case 0xA8: Reset(B, 5); break; //RES 5,B
		case 0xA9: Reset(C, 5); break; //RES 5,C
		case 0xAA: Reset(D, 5); break; //RES 5,D
		case 0xAB: Reset(E, 5); break; //RES 5,E
		case 0xAC: Reset(H, 5); break; //RES 5,H
		case 0xAD: Reset(L, 5); break; //RES 5,L
		case 0xAE: write(HL, M_Write_Helper(&CPU::Reset, 5)); break; //RES 5,(HL)
		case 0xAF: Reset(A, 5); break; //RES 5,A
		case 0xB0: Reset(B, 6); break; //RES 6,B
		case 0xB1: Reset(C, 6); break; //RES 6,C
		case 0xB2: Reset(D, 6); break; //RES 6,D
		case 0xB3: Reset(E, 6); break; //RES 6,E
		case 0xB4: Reset(H, 6); break; //RES 6,H
		case 0xB5: Reset(L, 6); break; //RES 6,L
		case 0xB6: write(HL, M_Write_Helper(&CPU::Reset, 6)); break; //RES 6,(HL)
		case 0xB7: Reset(A, 6); break; //RES 6,A
		case 0xB8: Reset(B, 7); break; //RES 7,B
		case 0xB9: Reset(C, 7); break; //RES 7,C
		case 0xBA: Reset(D, 7); break; //RES 7,D
		case 0xBB: Reset(E, 7); break; //RES 7,E
		case 0xBC: Reset(H, 7); break; //RES 7,H
		case 0xBD: Reset(L, 7); break; //RES 7,L
		case 0xBE: write(HL, M_Write_Helper(&CPU::Reset, 7)); break; //RES 7,(HL)
		case 0xBF: Reset(A, 7); break; //RES 7,A

		case 0xC0: Set(B, 0); break; //SET 0,B
		case 0xC1: Set(C, 0); break; //SET 0,C
		case 0xC2: Set(D, 0); break; //SET 0,D
		case 0xC3: Set(E, 0); break; //SET 0,E
		case 0xC4: Set(H, 0); break; //SET 0,H
		case 0xC5: Set(L, 0); break; //SET 0,L
		case 0xC6: write(HL, M_Write_Helper(&CPU::Set, 0)); break; //SET 0,(HL)
		case 0xC7: Set(A, 0); break; //SET 0,A
		case 0xC8: Set(B, 1); break; //SET 1,B
		case 0xC9: Set(C, 1); break; //SET 1,C
		case 0xCA: Set(D, 1); break; //SET 1,D
		case 0xCB: Set(E, 1); break; //SET 1,E
		case 0xCC: Set(H, 1); break; //SET 1,H
		case 0xCD: Set(L, 1); break; //SET 1,L
		case 0xCE: write(HL, M_Write_Helper(&CPU::Set, 1)); break; //SET 1,(HL)
		case 0xCF: Set(A, 1); break; //SET 1,A
		case 0xD0: Set(B, 2); break; //SET 2,B
		case 0xD1: Set(C, 2); break; //SET 2,C
		case 0xD2: Set(D, 2); break; //SET 2,D
		case 0xD3: Set(E, 2); break; //SET 2,E
		case 0xD4: Set(H, 2); break; //SET 2,H
		case 0xD5: Set(L, 2); break; //SET 2,L
		case 0xD6: write(HL, M_Write_Helper(&CPU::Set, 2)); break; //SET 2,(HL)
		case 0xD7: Set(A, 2); break; //SET 2,A
		case 0xD8: Set(B, 3); break; //SET 3,B
		case 0xD9: Set(C, 3); break; //SET 3,C
		case 0xDA: Set(D, 3); break; //SET 3,D
		case 0xDB: Set(E, 3); break; //SET 3,E
		case 0xDC: Set(H, 3); break; //SET 3,H
		case 0xDD: Set(L, 3); break; //SET 3,L
		case 0xDE: write(HL, M_Write_Helper(&CPU::Set, 3)); break; //SET 3,(HL)
		case 0xDF: Set(A, 3); break; //SET 3,A
		case 0xE0: Set(B, 4); break; //SET 4,B
		case 0xE1: Set(C, 4); break; //SET 4,C
		case 0xE2: Set(D, 4); break; //SET 4,D
		case 0xE3: Set(E, 4); break; //SET 4,E
		case 0xE4: Set(H, 4); break; //SET 4,H
		case 0xE5: Set(L, 4); break; //SET 4,L
		case 0xE6: write(HL, M_Write_Helper(&CPU::Set, 4)); break; //SET 4,(HL)
		case 0xE7: Set(A, 4); break; //SET 4,A
		case 0xE8: Set(B, 5); break; //SET 5,B
		case 0xE9: Set(C, 5); break; //SET 5,C
		case 0xEA: Set(D, 5); break; //SET 5,D
		case 0xEB: Set(E, 5); break; //SET 5,E
		case 0xEC: Set(H, 5); break; //SET 5,H
		case 0xED: Set(L, 5); break; //SET 5,L
		case 0xEE: write(HL, M_Write_Helper(&CPU::Set, 5)); break; //SET 5,(HL)
		case 0xEF: Set(A, 5); break; //SET 5,A
		case 0xF0: Set(B, 6); break; //SET 6,B
		case 0xF1: Set(C, 6); break; //SET 6,C
		case 0xF2: Set(D, 6); break; //SET 6,D
		case 0xF3: Set(E, 6); break; //SET 6,E
		case 0xF4: Set(H, 6); break; //SET 6,H
		case 0xF5: Set(L, 6); break; //SET 6,L
		case 0xF6: write(HL, M_Write_Helper(&CPU::Set, 6)); break; //SET 6,(HL)
		case 0xF7: Set(A, 6); break; //SET 6,A
		case 0xF8: Set(B, 7); break; //SET 7,B
		case 0xF9: Set(C, 7); break; //SET 7,C
		case 0xFA: Set(D, 7); break; //SET 7,D
		case 0xFB: Set(E, 7); break; //SET 7,E
		case 0xFC: Set(H, 7); break; //SET 7,H
		case 0xFD: Set(L, 7); break; //SET 7,L
		case 0xFE: write(HL, M_Write_Helper(&CPU::Set, 7)); break; //SET 7,(HL)
		case 0xFF: Set(A, 7); break; //SET 7,A

		default: fmt::printf("Unimplemented CB: 0x%02X\n", opcode); break;
	}
}

u8 CPU::FetchOpcode(u16 counter) {
	if(counter > 0xFFFF) throw "[CPU::FetchOpcode] counter is bigger than 0xFFFF";  //this should never happen

	return GetLEBytes<u8>(counter);
}

bool CPU::CheckZero() {
	return F.Z;
}

bool CPU::CheckNegative() {
	return F.N;
}

bool CPU::CheckHalfCarry() {
	return F.HC;
}

bool CPU::CheckCarry() {
	return F.C;
}

u8 CPU::SetFlags(u16 flags, u16 ans, u8 old, u8 diff) {
	if(flags & Carry)		SetCarry(ans);
	if(flags & HalfCarry)	SetHalfCarry(u8(ans), old, diff);
	if(flags & Zero)		SetZero((ans & 0xff) == 0);

	return u8(ans);
}

u8 CPU::SetCarry(u16 ans) {
	F.C = (ans > 0xff);
	return u8(ans);
}

u8 CPU::SetHalfCarry(u8 ans, u8 old, u8 diff) {
	F.HC = ((old ^ diff ^ ans) & 0x10) != 0;
	return ans;
}

u8 CPU::SetZero(int ans) {
	F.Z = (ans & 0xff) == 0;
	return ans;
}

void CPU::SetCarry(bool val) {
	F.C = val;
}

void CPU::SetHalfCarry(bool val) {
	F.HC = val;
}

void CPU::SetNegative(bool val) {
	F.N = val;
}

void CPU::SetZero(bool val) {
	F.Z = val;
}

void CPU::write(u16 loc, u8 value) {
	gb.mem.Write(loc, value);
}

void CPU::write(u16 loc, u16 value) {
	gb.mem.Write(loc, u8(value & 0xff));
	gb.mem.Write(++loc, u8(value >> 8));
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
	for (size_t cycles = 0; cycles < sizeof(T); ++cycles) {
		gb.scheduler.newMCycle();
	}

	T ret = 0, amount = sizeof(T);
	//while(amount > 0) ret += u8(gb.mem.Read((increase) ? addr++ : addr + (sizeof(T) - amount))) << ((sizeof(T) - amount--) * 8);
	
	while (amount > 0) {
		u16 newAddr = 0;
		u16 currentByte = (sizeof(T) - amount);

		if (increase) {
			newAddr = addr++;
		}
		else {
			newAddr = addr + currentByte;
		}
	
		ret += gb.mem.Read(newAddr) << (currentByte * 8);
		
		--amount;
	}
	
	return ret;
}

void CPU::Add(u16 in) {
	//u16 temp = setFlags(Negative_Unset | HalfCarry | Carry, L + (in & 0xff), L, (in & 0xff));
	u16 temp = L + (in & 0xff); 
	L = u8(temp);
	H = SetFlags(HalfCarry | Carry, H + (temp >> 8) + (in >> 8), H, (in >> 8));
	SetNegative(false);
}

void CPU::Add(u8 in, bool carry) {
	A = SetFlags(Zero | HalfCarry | Carry, A + in + carry, A, in);
	SetNegative(false);
}

void CPU::Sub(u8 in, bool carry) {
	u8 oldA = A;
	A = SetFlags(Zero | HalfCarry | Carry, A - in - carry, A, in);
	SetHalfCarry((oldA & 0xF) < (in & 0xF) + carry);
	SetNegative(true);
}

void CPU::And(u8 in) {
	A = SetFlags(Zero, A & in);
	SetCarry(false);
	SetHalfCarry(true);
	SetNegative(false);
}

void CPU::Xor(u8 in) {
	A = SetFlags(Zero, A ^ in);
	SetCarry(false);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::Or(u8 in) {
	A = SetFlags(Zero, A | in);
	SetCarry(false);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::Compare(u8 in) {
	SetFlags(Zero | HalfCarry | Carry, A - in, A, in);
	SetHalfCarry((A & 0xF) < (in & 0xF));
	SetNegative(true);
}

void CPU::Increase(u8& reg) {
	reg = SetFlags(Zero | HalfCarry, reg + 1, reg, 1);
	SetNegative(false);
}

void CPU::Decrease(u8& reg) {
	reg = SetFlags(Zero | HalfCarry, reg - 1, reg, 1);
	SetNegative(true);
}

void CPU::Increase(u16& reg) {
	reg++;
}

void CPU::Decrease(u16& reg) {
	reg--;
}

void CPU::RotateLeft(u8& reg, bool carry) {
	bool bit0 = (reg & 0x80);
	reg = SetFlags(Zero, u8(carry ? CheckCarry() : bit0) | (reg << 1));
	SetCarry(bit0);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::RotateRight(u8& reg, bool carry) {
	bool bit0 = (reg & 0x1);
	reg = SetZero(((carry ? CheckCarry() : bit0) << 7) | (reg >> 1));
	SetCarry(bit0);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::ShiftLeftArithmetic(u8& reg) {
	SetCarry((reg & 0x80) == 0x80);
	reg = SetZero(reg << 1);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::ShiftRightArithmetic(u8& reg) {
	SetCarry((reg & 0x1) == 0x1);
	reg = SetZero((reg & 0x80) | (reg >> 1));
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::Swap(u8& reg) {
	reg = SetZero((reg << 4) | (reg >> 4));
	SetCarry(false);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::ShiftRightLogical(u8& reg) {
	SetCarry((reg & 0x1) == 0x1); // set carry before reg gets modified
	reg = SetZero(reg >> 1);
	SetHalfCarry(false);
	SetNegative(false);
}

void CPU::Bit(u8 reg, u8 bit) {
	SetZero((reg & (1 << bit)) == 0);
	SetHalfCarry(true);
	SetNegative(false);
}

void CPU::Set(u8& reg, u8 bit) {
	reg |= (1 << bit);
}

void CPU::Reset(u8& reg, u8 bit) {
	reg &= ~(1 << bit);
}

void CPU::Load(u8& loc, u8 val) {
	loc = val;
}

void CPU::Load(u16& loc, u16 val) {
	loc = val;
}

void CPU::Push(u16& reg_pair) {
	gb.scheduler.newMCycle();
	SP -= 2;
	write(SP, reg_pair);
}

void CPU::Pop(u16& reg_pair) {
	reg_pair = GetLEBytes<u16>(SP);
	SP += 2;
}

void CPU::Jump(u16 loc, bool cond) {
	if (cond) {
		gb.scheduler.newMCycle();
		PC = loc;
	}
}

void CPU::JumpRelative(s8 offset, bool cond) {
	if (cond) {
		gb.scheduler.newMCycle();
		PC += offset;
	}
}

void CPU::Call(u16 loc, bool cond) {
	if(cond) {
		//gb.scheduler.newMCycle(); push already runs a m-cycle so this isn't needed
		Push(PC);
		PC = loc;
	}
}

void CPU::Ret() {
	Pop(PC);
	gb.scheduler.newMCycle();
}

void CPU::Rst(u8 loc) {
	Call(loc);
}