#define Cy 0b00010000
#define Cys 0b0001
#define HC 0b00100000
#define HCs 0b0010
#define N 0b01000000
#define Ns 0b0100
#define Z 0b10000000
#define Zs 0b1000

#define FLAGS_PATTERN "%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0')

#include "CPU.h"

struct instruction {
  int cycles;
  int pc;
  std::string_view name;
  int arg;
};

std::array<instruction, 256> const instTable {{
//             0                              1                             2                 3                             4                         5                           6                           7                         8                        9                       a                           b                 c                         d                       e                     f
    {4, 1, "NOP",               0}, {10, 3, "LD BC,$%hX", 0}, { 7, 1, "LD (BC),A",      0}, { 5, 1, "INC BC",    0}, { 5, 1, "INC B",        0}, { 5, 1, "DEC B",     0}, { 7, 2, "LD B,$%hhX",    0}, {4, 1, "RLCA",      0}, {4, 0, "LD ($%hX),SP",   0}, {10, 1, "ADD HL,BC", 0}, { 7, 1, "LD A,(BC)",  0}, { 5, 1, "DEC BC",  0}, { 5, 1, "INC C",      0}, { 5, 1, "DEC C",    0}, {7, 2, "LD,$%hhX",    0}, {4, 1, "RRCA",    0}, //00
    {4, 0, "STOP",              0}, {10, 3, "LD DE,$%hX", 0}, { 7, 1, "LD (BC),A",      0}, { 5, 1, "INC DE",    0}, { 5, 1, "INC D",        0}, { 5, 1, "DEC D",     0}, { 7, 2, "LD D,$%hhX",    0}, {4, 1, "RLA",       0}, {4, 0, "JR %hhd",        0}, {10, 1, "ADD HL,DE", 0}, { 7, 1, "LD A,(DE)",  0}, { 5, 1, "DEC DE",  0}, { 5, 1, "INC E",      0}, { 5, 1, "DEC E",    0}, {7, 2, "LD,$%hhX",    0}, {4, 1, "RRA",     0}, //10
    {4, 0, "JR NZ,%hhd",        0}, {10, 3, "LD HL,$%hX", 0}, {16, 3, "LD (HL+),A",     0}, { 5, 1, "INC HL",    0}, { 5, 1, "INC H",        0}, { 5, 1, "DEC H",     0}, { 7, 2, "LD H,$%hhX",    0}, {4, 1, "DAA",       0}, {4, 0, "JR Z,%hhd",      0}, {10, 1, "ADD HL,HL", 0}, {16, 3, "LD A,(HL+)", 0}, { 5, 1, "DEC HL",  0}, { 5, 1, "INC L",      0}, { 5, 1, "DEC L",    0}, {7, 2, "LD,$%hhX",    0}, {4, 1, "CPL",     0}, //20
    {4, 0, "JR NC,%hhd",        0}, {10, 3, "LD SP,$%hX", 0}, {13, 3, "LD (HL-),A",     0}, { 5, 1, "INC SP",    0}, { 5, 1, "INC (HL)",     0}, { 5, 1, "DEC (HL)",  0}, {10, 2, "LD (HL),$%hhX", 0}, {4, 1, "SCF",       0}, {4, 0, "JR C,%hhd",      0}, {10, 1, "ADD HL,SP", 0}, {13, 3, "LD A,(HL-)", 0}, { 5, 1, "DEC SP",  0}, { 5, 1, "INC A",      0}, { 5, 1, "DEC A",    0}, {7, 2, "LD,$%hhX",    0}, {4, 1, "CCF",     0}, //30
    {5, 1, "LD B,B",            0}, { 5, 1, "LD B,C",     0}, { 5, 1, "LD B,D",         0}, { 5, 1, "LD B,E",    0}, { 5, 1, "LD B,H",       0}, { 5, 1, "LD B,L",    0}, { 7, 1, "LD B,(HL)",     0}, {5, 1, "LD B,A",    0}, {5, 1, "LD C,B",         0}, { 5, 1, "LD C,C",    0}, { 5, 1, "LD C,D",     0}, { 5, 1, "LD C,E",  0}, { 5, 1, "LD C,H",     0}, { 5, 1, "LD C,L",   0}, {7, 1, "LD C,(HL)",   0}, {5, 1, "LD C,A",  0}, //40
    {5, 1, "LD D,B",            0}, { 5, 1, "LD D,C",     0}, { 5, 1, "LD D,D",         0}, { 5, 1, "LD D,E",    0}, { 5, 1, "LD D,H",       0}, { 5, 1, "LD D,L",    0}, { 7, 1, "LD D,(HL)",     0}, {5, 1, "LD D,A",    0}, {5, 1, "LD E,B",         0}, { 5, 1, "LD E,C",    0}, { 5, 1, "LD E,D",     0}, { 5, 1, "LD E,E",  0}, { 5, 1, "LD E,H",     0}, { 5, 1, "LD E,L",   0}, {7, 1, "LD E,(HL)",   0}, {5, 1, "LD E,A",  0}, //50
    {5, 1, "LD H,B",            0}, { 5, 1, "LD H,C",     0}, { 5, 1, "LD H,D",         0}, { 5, 1, "LD H,E",    0}, { 5, 1, "LD H,H",       0}, { 5, 1, "LD H,L",    0}, { 7, 1, "LD H,(HL)",     0}, {5, 1, "LD H,A",    0}, {5, 1, "LD L,B",         0}, { 5, 1, "LD L,C",    0}, { 5, 1, "LD L,D",     0}, { 5, 1, "LD L,E",  0}, { 5, 1, "LD L,H",     0}, { 5, 1, "LD L,L",   0}, {7, 1, "LD L,(HL)",   0}, {5, 1, "LD L,A",  0}, //60
    {7, 1, "LD (HL),B",         0}, { 7, 1, "LD (HL),C",  0}, { 7, 1, "LD (HL),D",      0}, { 7, 1, "LD (HL),E", 0}, { 7, 1, "LD (HL),H",    0}, { 7, 1, "LD (HL),L", 0}, { 7, 1, "HALT",          0}, {7, 1, "LD (HL),A", 0}, {5, 1, "LD A,B",         0}, { 5, 1, "LD A,C",    0}, { 5, 1, "LD A,D",     0}, { 5, 1, "LD A,E",  0}, { 5, 1, "LD A,H",     0}, { 5, 1, "LD A,L",   0}, {7, 1, "LD A,(HL)",   0}, {5, 1, "LD A,A",  0}, //70
    {4, 1, "ADD A,B",           0}, { 4, 1, "ADD A,C",    0}, { 4, 1, "ADD A,D",        0}, { 4, 1, "ADD A,E",   0}, { 4, 1, "ADD A,H",      0}, { 4, 1, "ADD A,L",   0}, { 7, 1, "ADD A,(HL)",    0}, {4, 1, "ADD A,A",   0}, {4, 1, "ADC A,B,",       0}, { 4, 1, "ADC A,C",   0}, { 4, 1, "ADC A,D",    0}, { 4, 1, "ADC A,E", 0}, { 4, 1, "ADC A,H",    0}, { 4, 1, "ADC A,L",  0}, {7, 1, "ADC A,(HL)",  0}, {4, 1, "ADC A,A", 0}, //80
    {4, 1, "SUB A,B",           0}, { 4, 1, "SUB A,C",    0}, { 4, 1, "SUB A,D",        0}, { 4, 1, "SUB A,E",   0}, { 4, 1, "SUB A,H",      0}, { 4, 1, "SUB A,L",   0}, { 7, 1, "SUB A,(HL)",    0}, {4, 1, "SUB A,A",   0}, {4, 1, "SBC A,B,",       0}, { 4, 1, "SBC A,C",   0}, { 4, 1, "SBC A,D",    0}, { 4, 1, "SBC A,E", 0}, { 4, 1, "SBC A,H",    0}, { 4, 1, "SBC A,L",  0}, {7, 1, "SBC A,(HL)",  0}, {4, 1, "SBC A,A", 0}, //90
    {4, 1, "AND A,B",           0}, { 4, 1, "AND A,C",    0}, { 4, 1, "AND A,D",        0}, { 4, 1, "AND A,E",   0}, { 4, 1, "AND A,H",      0}, { 4, 1, "AND A,L",   0}, { 7, 1, "AND A,(HL)",    0}, {4, 1, "AND A,A",   0}, {4, 1, "XOR A,B,",       0}, { 4, 1, "XOR A,C",   0}, { 4, 1, "XOR A,D",    0}, { 4, 1, "XOR A,E", 0}, { 4, 1, "XOR A,H",    0}, { 4, 1, "XOR A,L",  0}, {7, 1, "XOR A,(HL)",  0}, {4, 1, "XOR A,A", 0}, //a0
    {4, 1, "OR A,B",            0}, { 4, 1, "OR A,C",     0}, { 4, 1, "OR A,D",         0}, { 4, 1, "OR A,E",    0}, { 4, 1, "OR A,H",       0}, { 4, 1, "OR A,L",    0}, { 7, 1, "OR A,(HL)",     0}, {4, 1, "OR A,A",    0}, {4, 1, "CP A,B",         0}, { 4, 1, "CP A,C",    0}, { 4, 1, "CP A,D",     0}, { 4, 1, "CP A,E",  0}, { 4, 1, "CP A,H",     0}, { 4, 1, "CP A,L",   0}, {7, 1, "CP A,(HL)",   0}, {4, 1, "CP A,A",  0}, //b0
    {5, 0, "RET NZ",            0}, {10, 1, "POP BC",     0}, {10, 0, "JP NZ,$%hX",     0}, {10, 0, "JP $%hX",   0}, {11, 0, "CALL NZ,$%hX", 0}, {11, 1, "PUSH BC",   0}, { 7, 2, "ADD A,$%hhX",   0}, {5, 0, "RST $00",   0}, {5, 0, "RET Z",          0}, { 4, 0, "RET",       0}, {10, 0, "JP Z,$hX",   0}, { 4, 2, "CB:",     0}, {11, 0, "CALL Z,$hX", 0}, {11, 0, "CALL $hX", 0}, {7, 2, "ADC A,$%hhX", 0}, {5, 0, "RST $08", 0}, //c0
    {5, 0, "RET NC",            0}, {10, 1, "POP DE",     0}, {10, 0, "JP NC,$%hX",     0}, {10, 2, "",          0}, {11, 0, "CALL NC,$%hX", 0}, {11, 1, "PUSH DE",   0}, { 7, 2, "SUB A,$%hhX",   0}, {5, 0, "RST $10",   0}, {5, 0, "RET C",          0}, { 4, 0, "RETI",      0}, {10, 0, "JP C,$hX",   0}, {10, 2, "",        0}, {11, 0, "CALL C,$hX", 0}, { 4, 0, "",         0}, {7, 2, "SBC A,$%hhX", 0}, {5, 0, "RST $18", 0}, //d0
    {5, 0, "LD ($FF00+%hhX),A", 0}, {10, 1, "POP HL",     0}, {10, 0, "LD ($FF00+C),A", 0}, {18, 1, "",          0}, {11, 0, "",             0}, {11, 1, "PUSH HL",   0}, { 7, 2, "AND A,$%hhX",   0}, {5, 0, "RST $20",   0}, {5, 0, "ADD SP,%hhd",    0}, { 5, 0, "JP HL",     0}, {10, 0, "LD ($hX),A", 0}, { 4, 1, "",        0}, {11, 0, "",           0}, { 4, 0, "",         0}, {7, 2, "XOR A,$%hhX", 0}, {5, 0, "RST $28", 0}, //e0
    {5, 0, "LD A,($FF00+%hhX)", 0}, {10, 1, "POP AF",     0}, {10, 0, "LD A,($FF00+C)", 0}, { 4, 1, "DI",        0}, {11, 0, "",             0}, {11, 1, "PUSH AF",   0}, { 7, 2, "OR A,$%hhX",    0}, {5, 0, "RST $30",   0}, {5, 0, "LD HL, SP+%hhd", 0}, { 5, 1, "LD SP,HL",  0}, {10, 0, "LD A,($hX)", 0}, { 4, 1, "EI",      0}, {11, 0, "",           0}, { 4, 0, "",         0}, {7, 2, "CP A,$%hhX",  0}, {5, 0, "RST $38", 0},  //f0
}};

CPU::CPU() { //reset everything
    PC = 0x100; //non-bios mode
    SP = 0;

    log = fopen("log.txt", "w");
}

void CPU::emulateOpcode() {
    opcode = memory[PC];
    cycBefore = cycles;
    M = &memory[HL];

    handlePrint();

    switch(opcode) {
        //calls
        case 0xC3: jp(); break;
        case 0xC2: jp(Z, false); break;
        case 0xD2: jp(Cy, false); break;
        case 0xCA: jp(Z, true); break;
        case 0xDA: jp(Cy, true); break;

        case 0x20: jr(Z, false); break;

        //8-bit loads
        case 0x02: ld8(memory[BC], A); break;
        case 0x12: ld8(memory[DE], A); break;
        case 0x22: ld8(memory[++HL], A); break;
        case 0x32: ld8(memory[--HL], A); break;
        case 0x0A: ld8(A, memory[BC]); break;
        case 0x1A: ld8(A, memory[DE]); break;
        case 0x2A: ld8(A, memory[++HL]); break;
        case 0x3A: ld8(A, memory[--HL]); break;

        case 0x06: ld8(B, getNextBytes(1)); break;
        case 0x0E: ld8(C, getNextBytes(1)); break;
        case 0x16: ld8(D, getNextBytes(1)); break;
        case 0x1E: ld8(E, getNextBytes(1)); break;
        case 0x26: ld8(H, getNextBytes(1)); break;
        case 0x2E: ld8(L, getNextBytes(1)); break;
        case 0x36: ld8(*M, getNextBytes(1)); break;
        case 0x3E: ld8(A, getNextBytes(1)); break;
        case 0x40: ld8(B, B); break;
        case 0x41: ld8(B, C); break;
        case 0x42: ld8(B, D); break;
        case 0x43: ld8(B, E); break;
        case 0x44: ld8(B, H); break;
        case 0x45: ld8(B, L); break;
        case 0x46: ld8(B, *M); break;
        case 0x47: ld8(B, A); break;
        case 0x48: ld8(C, B); break;
        case 0x49: ld8(C, C); break;
        case 0x4A: ld8(C, D); break;
        case 0x4B: ld8(C, E); break;
        case 0x4C: ld8(C, H); break;
        case 0x4D: ld8(C, L); break;
        case 0x4E: ld8(C, *M); break;
        case 0x4F: ld8(C, A); break;
        case 0x50: ld8(D, B); break;
        case 0x51: ld8(D, C); break;
        case 0x52: ld8(D, D); break;
        case 0x53: ld8(D, E); break;
        case 0x54: ld8(D, H); break;
        case 0x55: ld8(D, L); break;
        case 0x56: ld8(D, *M); break;
        case 0x57: ld8(D, A); break;
        case 0x58: ld8(E, B); break;
        case 0x59: ld8(E, C); break;
        case 0x5A: ld8(E, D); break;
        case 0x5B: ld8(E, E); break;
        case 0x5C: ld8(E, H); break;
        case 0x5D: ld8(E, L); break;
        case 0x5E: ld8(E, *M); break;
        case 0x5F: ld8(E, A); break;
        case 0x60: ld8(H, B); break;
        case 0x61: ld8(H, C); break;
        case 0x62: ld8(H, D); break;
        case 0x63: ld8(H, E); break;
        case 0x64: ld8(H, H); break;
        case 0x65: ld8(H, L); break;
        case 0x66: ld8(H, *M); break;
        case 0x67: ld8(H, A); break;
        case 0x68: ld8(L, B); break;
        case 0x69: ld8(L, C); break;
        case 0x6A: ld8(L, D); break;
        case 0x6B: ld8(L, E); break;
        case 0x6C: ld8(L, H); break;
        case 0x6D: ld8(L, L); break;
        case 0x6E: ld8(L, *M); break;
        case 0x6F: ld8(L, A); break;
        case 0x70: ld8(*M, B); break;
        case 0x71: ld8(*M, C); break;
        case 0x72: ld8(*M, D); break;
        case 0x73: ld8(*M, E); break;
        case 0x74: ld8(*M, H); break;
        case 0x75: ld8(*M, L); break;
        case 0x77: ld8(*M, A); break;

        case 0xE0: ld8(A, memory[(0xFF00 + getNextBytes(1))]); break;
        case 0xE2: ld8(A, memory[(0xFF00 + C)]); break;
        case 0xEA: ld8(A, memory[getNextBytes(2)]); break;
        case 0xF0: ld8(memory[(0xFF00 + getNextBytes(1))], A); break;
        case 0xF2: ld8(memory[(0xFF00 + C)], A); break;
        case 0xFA: ld8(memory[getNextBytes(2)], A); break;

        //8-bit math
        case 0x04: math(B, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x14: math(C, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x24: math(D, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x34: math(E, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x0C: math(H, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x1C: math(L, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x2C: math(*M, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x3C: math(A, 1, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x05: math(B, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x15: math(C, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x25: math(D, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x35: math(E, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x0D: math(H, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x1D: math(L, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x2D: math(*M, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x3D: math(A, 1, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;

        case 0x80: math(A, B, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x81: math(A, C, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x82: math(A, D, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x83: math(A, E, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x84: math(A, H, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x85: math(A, L, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x86: math(A, *M, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x87: math(A, A, std::plus<u16>(), Z|HC|Cy|Ns); break;
        case 0x88: math(A, B, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x89: math(A, C, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8A: math(A, D, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8B: math(A, E, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8C: math(A, H, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8D: math(A, L, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8E: math(A, *M, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x8F: math(A, A, std::plus<u16>(), Z|HC|Cy|Ns, true); break;
        case 0x90: math(A, B, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x91: math(A, C, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x92: math(A, D, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x93: math(A, E, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x94: math(A, H, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x95: math(A, L, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x96: math(A, *M, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x97: math(A, A, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0x98: math(A, B, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x99: math(A, C, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9A: math(A, D, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9B: math(A, E, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9C: math(A, H, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9D: math(A, L, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9E: math(A, *M, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0x9F: math(A, A, std::minus<u16>(), Z|N|HC|Cy|Ns, true, true); break;
        case 0xA0: math(A, B, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA1: math(A, C, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA2: math(A, D, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA3: math(A, E, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA4: math(A, H, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA5: math(A, L, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA6: math(A, *M, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA7: math(A, A, std::bit_and<u16>(), Z|HC|Ns|HCs|Cys); break;
        case 0xA8: math(A, B, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xA9: math(A, C, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAA: math(A, D, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAB: math(A, E, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAC: math(A, H, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAD: math(A, L, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAE: math(A, *M, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xAF: math(A, A, std::bit_xor<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB0: math(A, B, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB1: math(A, C, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB2: math(A, D, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB3: math(A, E, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB4: math(A, H, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB5: math(A, L, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB6: math(A, *M, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB7: math(A, A, std::bit_or<u16>(), Z|Ns|HCs|Cys); break;
        case 0xB8: math(A, B, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xB9: math(A, C, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBA: math(A, D, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBB: math(A, E, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBC: math(A, H, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBD: math(A, L, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBE: math(A, *M, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;
        case 0xBF: math(A, A, std::minus<u16>(), Z|N|HC|Cy|Ns, false, true); break;

        //16-bit loads
        case 0x01: ld16(BC, getNextBytes(2)); break;
        case 0x11: ld16(DE, getNextBytes(2)); break;
        case 0x21: ld16(HL, getNextBytes(2)); break;
        case 0x31: ld16(SP, getNextBytes(2)); break;
        case 0x08: memcpy(&memory[getNextBytes(2)], &SP, sizeof(SP)); break; //little-endian
        case 0xF9: ld16(SP, HL); break;

        case 0xC1: BC = pop();
        case 0xD1: DE = pop();
        case 0xE1: HL = pop();
        case 0xF1: AF = pop();

        case 0xC5: push(BC);
        case 0xD5: push(DE);
        case 0xE5: push(HL);
        case 0xF5: push(AF);

        //16-bit math
        case 0xE8: math(SP, getNextBytes(1), std::plus<u16>(), HC|Cy); break;
        case 0xF8: math(SP, getNextBytes(1), std::plus<u16>(), HC|Cy); ld16(HL, SP); break;

        case 0x09: add16(HL, BC); break;
        case 0x19: add16(HL, DE); break;
        case 0x29: add16(HL, HL); break;
        case 0x39: add16(HL, SP); break;

        case 0x03: inc16(BC); break;
        case 0x13: inc16(DE); break;
        case 0x23: inc16(HL); break;
        case 0x33: inc16(SP); break;

        case 0x0B: dec16(BC); break;
        case 0x1B: dec16(DE); break;
        case 0x2B: dec16(HL); break;
        case 0x3B: dec16(SP); break;

        //cb prefix
        case 0xCB: {
            switch(getNextBytes(1)) {
                case 0x40: bit(0, B); break;
                case 0x41: bit(0, C); break;
                case 0x42: bit(0, D); break;
                case 0x43: bit(0, E); break;
                case 0x44: bit(0, H); break;
                case 0x45: bit(0, L); break;
                case 0x46: bit(0, *M); break;
                case 0x47: bit(0, A); break;
                case 0x48: bit(1, B); break;
                case 0x49: bit(1, C); break;
                case 0x4A: bit(1, D); break;
                case 0x4B: bit(1, E); break;
                case 0x4C: bit(1, H); break;
                case 0x4D: bit(1, L); break;
                case 0x4E: bit(1, *M); break;
                case 0x4F: bit(1, A); break;
                case 0x50: bit(2, B); break;
                case 0x51: bit(2, C); break;
                case 0x52: bit(2, D); break;
                case 0x53: bit(2, E); break;
                case 0x54: bit(2, H); break;
                case 0x55: bit(2, L); break;
                case 0x56: bit(2, *M); break;
                case 0x57: bit(2, A); break;
                case 0x58: bit(3, B); break;
                case 0x59: bit(3, C); break;
                case 0x5A: bit(3, D); break;
                case 0x5B: bit(3, E); break;
                case 0x5C: bit(3, H); break;
                case 0x5D: bit(3, L); break;
                case 0x5E: bit(3, *M); break;
                case 0x5F: bit(3, A); break;
                case 0x60: bit(4, B); break;
                case 0x61: bit(4, C); break;
                case 0x62: bit(4, D); break;
                case 0x63: bit(4, E); break;
                case 0x64: bit(4, H); break;
                case 0x65: bit(4, L); break;
                case 0x66: bit(4, *M); break;
                case 0x67: bit(4, A); break;
                case 0x68: bit(5, B); break;
                case 0x69: bit(5, C); break;
                case 0x6A: bit(5, D); break;
                case 0x6B: bit(5, E); break;
                case 0x6C: bit(5, H); break;
                case 0x6D: bit(5, L); break;
                case 0x6E: bit(5, *M); break;
                case 0x6F: bit(5, A); break;
                case 0x70: bit(6, B); break;
                case 0x71: bit(6, C); break;
                case 0x72: bit(6, D); break;
                case 0x73: bit(6, E); break;
                case 0x74: bit(6, H); break;
                case 0x75: bit(6, L); break;
                case 0x76: bit(6, *M); break;
                case 0x77: bit(6, A); break;
                case 0x78: bit(7, B); break;
                case 0x79: bit(7, C); break;
                case 0x7A: bit(7, D); break;
                case 0x7B: bit(7, E); break;
                case 0x7C: bit(7, H); break;
                case 0x7D: bit(7, L); break;
                case 0x7E: bit(7, *M); break;
                case 0x7F: bit(7, A); break;
            }
        } break;

        //misc
        case 0x00: break; //nop
        case 0x10: low_power = 2; break; //stop
        case 0xF3: IME = false; break; //di
        case 0xFB: IME = true; break; //ei
        case 0x76: low_power = 1; break; //halt
        case 0x27: daa(); break;
        case 0x37: setFlags(0, 0, 0, Cy|Ns|HCs|Cys); break; //scf
        case 0x2F: A = ~A; setFlags(0, 0, 0, N|HC|Ns|HCs); break; //cpl
        case 0x3F: F ^= 0x10; setFlags(0, 0, 0, Ns|HCs); break; //ccf

        default: printf("Unimplimented Opcode: 0x%02X", opcode); break;
    }

    cycles += instTable[opcode].cycles;
    PC += instTable[opcode].pc;

    if(memory[0xff02] == 0x81) {
        auto c = memory[0xff01];
        printf("%c", c);
        memory[0xff02] = 0x0;
    }
}

void CPU::handlePrint() {
    fprintf(log, "%04x: ", PC);

    auto qwqs = string(instTable[opcode].name);
    auto qwq = qwqs.c_str();
    switch(instTable[opcode].arg) {
      case 0: fprintf(log, qwq); break;
      case 1: fprintf(log, qwq, 0xFF00 + C); break;
      case 2: fprintf(log, qwq, 0xFF00 + getNextBytes(1)); break;
      case 3: fprintf(log, qwq, getNextBytes(2)); break;
    }

    fprintf(log, " | SP:%04x BC:%04x DE:%04x HL:%04x A:%02x F:", SP, BC, DE, HL, A);
    fprintf(log, FLAGS_PATTERN"\n", BYTE_TO_BINARY(F));
}

u16 CPU::getNextBytes(int amount) {
    switch(amount) {
        case 2: return (memory[PC+amount] << 8) + memory[PC+amount-1];
        case 1: return memory[PC+amount];
        default: return 0;
    }
}

u16 CPU::setFlags(u16 ans, u8 old, u8 diff, u8 flags) {
    F &= ~((flags & 0xF0) | (flags << 4)); //reset flags to be modified in order to properly set them

    if(flags & 0x8)         F |= flags & 0x80; //z
    else if(flags & 0x80)   F |= (ans & 0xff) ? 0x80 : 0;
    if(flags & 0x4)         F |= flags & 0x40; //n
    else if(flags & 0x40)   F |= parity(ans & 0xff) ? 0x40 : 0;
    if(flags & 0x2)         F |= flags & 0x20; //hc
    else if(flags & 0x20)   F |= ((old ^ diff ^ ans) & 0x10) == 0x10 ? 0x20 : 0;
    if(flags & 0x1)         F |= flags & 0x10; //cy
    else if(flags & 0x10)   F |= (ans > 0xff) ? 0x10 : 0;

    return ans;
}

template <typename T>
void CPU::math(T& reg, u8 addend, std::function<u16(u16, u8)> func, u8 flags, bool carry, bool sub) {
    reg = setFlags(func(reg, (addend+carry)), reg, (sub) ? ~addend : addend, flags);
}

void CPU::add16(u16& reg, u16 addend) {
    F &= ~(N|HC|Cy); //reset flags to be modified in order to properly set them

    u32 tmp = reg + addend;
    F |= ((reg ^ addend ^ tmp) & 0x1000) == 0x1000 ? 0x20 : 0; //hc
    F |= (tmp > 0xffff) ? 0x10 : 0; //cy
    reg = tmp;
}

void CPU::inc16(u16& reg) {
    reg++;
}

void CPU::dec16(u16& reg) {
    reg--;
}

void CPU::ld8(u8& reg, u8 data) {
    reg = data;
}

void CPU::ld16(u16& reg, u16 data) {
    reg = data;
}

void CPU::daa() {
}

void CPU::call(u8 cond, bool n) {
    if(checkFlags(cond) == n) {
        //push PC to stack
        PC = getNextBytes(2);
    }
}

void CPU::jp(u8 cond, bool n) {
    if(checkFlags(cond) == n)
        PC = getNextBytes(2);
}

void CPU::jr(u8 cond, bool n) {
    if(checkFlags(cond) == n)
        PC += char(getNextBytes(1));
}

void CPU::ret(u8 cond, bool n) {
    if(checkFlags(cond) == n)
        PC = pop();
}

void CPU::rst() {
    //interrupt stuff

    call();
}

void CPU::push(u16 value) {
    memory[--SP] = value >> 8;
    memory[--SP] = value & 0xff;
}

u16 CPU::pop() {
    return memory[SP++] + (memory[SP++] << 8);
}

void CPU::bit(int bitT, u8 byte) {
    setFlags((byte >> bitT) & 0x1, 0, 0, Z|HC|Ns|HCs);
}

bool CPU::checkFlags(u8 cond) {
    return ((F & cond) == cond);
}

bool CPU::parity(u16 ans) {
    int count = 0;

    for(auto i = 0; i < 8; i++)
        ((ans & (1 << i)) != 0) ? count++ : 0;

    return (count % 2 == 0); //need to fix later x d
}
