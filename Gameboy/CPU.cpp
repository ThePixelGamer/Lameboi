#include "CPU.h"

constexpr int cyclesTable[256] = {
//  0   1   2   3   4   5   6  7  8   9   a   b   c   d  e  f 
    4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //00
    4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //10
    4, 10, 16,  5,  5,  5,  7, 4, 4, 10, 16,  5,  5,  5, 7, 4, //20
    4, 10, 13,  5,  5,  5, 10, 4, 4, 10, 13,  5,  5,  5, 7, 4, //30
    5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //40
    5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //50
    5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //60
    7,  7,  7,  7,  7,  7,  7, 7, 5,  5,  5,  5,  5,  5, 7, 5, //70
    4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //80
    4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //90
    4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //a0
    4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //b0
    5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10,  4, 11, 11, 7, 5, //c0
    5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10, 10, 11,  4, 7, 5, //d0
    5, 10, 10, 18, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5, //e0
    5, 10, 10,  4, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5  //f0
};
constexpr int pcTable[256] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f 
    1, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //00
    0, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //10
    0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //20
    0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //70
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //80
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //90
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //a0
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //b0
    0, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, //c0
    0, 1, 0, 2, 0, 1, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, //d0
    0, 1, 0, 1, 0, 1, 2, 0, 0, 0, 0, 1, 0, 0, 2, 0, //e0
    0, 1, 0, 1, 0, 1, 2, 0, 0, 1, 0, 1, 0, 0, 2, 0  //f0
};

constexpr u8 bios[256] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

CPU::CPU() { //reset everything
    SP = AF = BC = DE = HL = 0;

    A = ((pu8) &AF) + 1;
    F = ((pu8) &AF);
    B = ((pu8) &BC) + 1;
    C = ((pu8) &BC);
    D = ((pu8) &DE) + 1;
    E = ((pu8) &DE);
    H = ((pu8) &HL) + 1;
    L = ((pu8) &HL);

    PC = 0;
}

void CPU::emulateOpcode() {
    opcode = memory[PC];
	cycBefore = cycles;
    //setup log file

    switch(opcode) {
        case 0x0: break;
        case 0x20: jr(0x80, getNextOffsetByte(1), false); break;
        case 0x21: ld16(HL, getNext2Bytes()); break;
        case 0x31: ld16(SP, getNext2Bytes()); break;

        case 0x06: ld8(B, getNextOffsetByte(1)); break;
        case 0x0E: ld8(C, getNextOffsetByte(1)); break;
        case 0x16: ld8(D, getNextOffsetByte(1)); break;
        case 0x1E: ld8(E, getNextOffsetByte(1)); break;
        case 0x26: ld8(H, getNextOffsetByte(1)); break;
        case 0x2E: ld8(L, getNextOffsetByte(1)); break;
        case 0x36: ld8(&memory[HL], getNextOffsetByte(1)); break;
        case 0x3E: ld8(A, getNextOffsetByte(1)); break;

        case 0x02: ld8(&memory[BC], *A); break;
        case 0x12: ld8(&memory[DE], *A); break;
        case 0x22: ld8(&memory[++HL], *A); break;
        case 0x32: ld8(&memory[--HL], *A); break;
        case 0x0A: ld8(A, memory[BC]); break;
        case 0x1A: ld8(A, memory[DE]); break;
        case 0x2A: ld8(A, memory[++HL]); break;
        case 0x3A: ld8(A, memory[--HL]); break;

        case 0xAE: math8(A, memory[HL], bit_xor<u16>(), 0b10000111); break;
        case 0xAF: math8(A, *A, bit_xor<u16>(), 0b10000111); break;

        case 0xCB: {
            switch(getNextOffsetByte(1)) {
                case 0x7C: bit(7, *H); break;
                case 0x7F: bit(7, memory[HL]); break;
            }
        } break;

        default: printf("Unimplimented Opcode: 0x%02X", opcode); break;
    }

	cycles += cyclesTable[opcode];
    PC += pcTable[opcode];
}

void CPU::nop() {}

void CPU::ld8(pu8 reg, u8 data) {
    *reg = data;
}

void CPU::math8(pu8 reg, u8 addend, function<u16(u8, u8)> func, u8 flags) {
    *reg = setFlags(func(*reg, addend), *reg, addend, flags);
}

void CPU::ld16(u16& reg, u16 data) {
    reg = data;
}

void CPU::add16() {
}

void CPU::inc16() {
}

void CPU::dec16() {
}

void CPU::daa() {
}

void CPU::scf() {
}

void CPU::cpl() {
}

void CPU::ccf() {
}

void CPU::halt() {
}

void CPU::ei() {
}

void CPU::di() {
}

void CPU::call() {
}

void CPU::jp() {
}

void CPU::jr(u8 cond, char data, bool n) {
    if(checkFlags(cond) == n)
        PC += data;
}

void CPU::ret() {
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
    return memory[SP++] + (memory[SP++] >> 8);
}

void CPU::bit(int bitT, u8 byte) {
    setFlags((byte >> bitT) & 0x1, 0, 0, 0b10100110);
}

void CPU::changeM(u8 value) {
    //if(loc >= vramStart && loc < (vramStart + 0x1C00)) {} was swapped out for a PPU SoC
    memory[HL] = value;
} 

bool CPU::checkFlags(u8 cond) {
    return ((*F & cond) == cond);
}

u8 CPU::setFlags(u16 ans, u8 old, u8 diff, u8 flags) {
    if(flags & 0x8) //z
        *F |= flags & 0x80;
    else
        (flags & 0x80) ? *F |= (ans & 0xff) ? 0x80 : 0 : 0;
    if(flags & 0x4) //n
        *F |= flags & 0x40;
    else
        (flags & 0x40) ? *F |= parity(ans & 0xff) ? 0x40 : 0 : 0;
    if(flags & 0x2) //hc
        *F |= flags & 0x20;
    else
        (flags & 0x20) ? *F |= ((old ^ diff ^ ans) & 0x10) == 0x10 ? 0x20 : 0 : 0;
    if(flags & 0x1) //cy
        *F |= flags & 0x10;
    else
        (flags & 0x10) ? *F |= (ans > 0xff) ? 0x10 : 0 : 0;

	return (ans & 0xff);
}

bool CPU::parity(u16 ans) {
    int count = 0;

    for (int i = 0; i < 8; i++)
        ((ans & (1 << i)) != 0) ? count++ : 0;

    return (count % 2 == 0); //need to fix later x d
}

u8 CPU::getNextOffsetByte(int offset) {
    return memory[PC+offset];
}

u16 CPU::getNext2Bytes() {
    return ((u16)(getNextOffsetByte(2) << 8)) + getNextOffsetByte(1);
}
