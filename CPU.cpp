#include "CPU.h"

constexpr auto A = 0x7;
constexpr auto B = 0x0;
constexpr auto C = 0x1;
constexpr auto D = 0x2;
constexpr auto E = 0x3;
constexpr auto H = 0x4;
constexpr auto L = 0x5;

constexpr void (CPU::*CPU::opcodeTable[256])();
constexpr int cyclesTable[256] = {
//  0   1   2   3   4   5   6  7  8   9   a   b   c   d  e  f 
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //0
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //1
	4, 10, 16,  5,  5,  5,  7, 4, 4, 10, 16,  5,  5,  5, 7, 4, //2
	4, 10, 13,  5,  5,  5, 10, 4, 4, 10, 13,  5,  5,  5, 7, 4, //3
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //4
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //5
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //6
	7,  7,  7,  7,  7,  7,  7, 7, 5,  5,  5,  5,  5,  5, 7, 5, //7
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //8
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //9
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //a
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //b
	5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10,  4, 11, 11, 7, 5, //c
	5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10, 10, 11,  4, 7, 5, //d
	5, 10, 10, 18, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5, //e
	5, 10, 10,  4, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5  //f
};
constexpr int pcTable[256] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f 
	1, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //0
	0, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //1
	0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //2
	0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //3
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //4
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //5
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //6
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //7
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //8
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //9
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //a
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //b
	0, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, //c
	0, 1, 0, 2, 0, 1, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, //d
	0, 1, 0, 1, 0, 1, 2, 0, 0, 0, 0, 1, 0, 0, 2, 0, //e
	0, 1, 0, 1, 0, 1, 2, 0, 0, 1, 0, 1, 0, 0, 2, 0  //f
};

CPU::CPU() { //reset everything
    uint8_t bios[256] = {
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
}

void CPU::emulateOpcode() {
	opcode = memory[pc];
		
    loc = ((uint16_t)(registers[H] << 8)) + registers[L];
	cycBefore = cycles;

    //setup log file

	(this->*opcodeTable[opcode])();
	cycles += cyclesTable[opcode];
	pc += pcTable[opcode];
}

void CPU::nop() {}

void CPU::ld16() {
	uint8_t reg = (opcode >> 3) & 0x6;
    loc = ((uint16_t)(memory[pc+2] << 8)) + memory[pc+1];
	if(reg == 0x6)
		sp = loc;
	else {
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
} 

void CPU::sta() {
	uint8_t reg = (opcode >> 3) & 0x2;
    loc = ((uint16_t)(registers[reg] << 8)) + registers[reg+1];
	changeM(registers[A]);
}

void CPU::lda() {
	uint8_t reg = (opcode >> 3) & 0x2;
    loc = ((uint16_t)(registers[reg] << 8)) + registers[reg+1];
	registers[A] = memory[loc];
}

void CPU::incx() {
	uint8_t reg = (opcode >> 3) & 0x6;
	
 	if(reg == 0x6) 
		sp += 1;
	else {
        loc = ((uint16_t)((registers[reg] << 8)) + registers[reg+1]) + 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
}

void CPU::decx() {
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) 
		sp -= 1;
	else {
        loc = ((uint16_t)((registers[reg] << 8)) + registers[reg+1]) - 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
}

void CPU::inc() {
	uint8_t reg = (opcode >> 3) & 0x7;
	bool c = cy;

	if(reg == 0x6)
		changeM(setCond(memory[loc] + 1, memory[loc], 1));
	else
		registers[reg] = setCond(registers[reg] + 1, registers[reg], 1);
		
	cy = c;
} 

void CPU::dec() {
	uint8_t reg = (opcode >> 3) & 0x7;
	bool c = cy;

	if(reg == 0x6)
		changeM(setCond(memory[loc] - 1, memory[loc], (~1 + 1))); 
	else
		registers[reg] = setCond(registers[reg] - 1, registers[reg], (~1 + 1));

	cy = c;
} 

void CPU::dad() {
	uint8_t reg = (opcode >> 3) & 0x6;
	uint32_t tmp = loc;

	if(reg == 0x6)
		tmp += sp;
	else
        tmp += ((uint16_t)(registers[reg] << 8)) + registers[reg+1];

	registers[L] = tmp & 0xff;
    registers[H] = (tmp >> 8) & 0xff;
	cy = (tmp > 0xffff);
} 

void CPU::rlca() {
	cy = registers[A] >> 7;
    registers[A] = ((uint8_t)(registers[A] << 1)) + cy;
}

void CPU::rrca() {
	cy = registers[A] & 0x1;
    registers[A] = ((uint8_t)(registers[A] >> 1)) + ((uint8_t)(cy << 7));
}

void CPU::rla() {
	uint8_t c = cy;
	cy = registers[A] >> 7; 
    registers[A] = ((uint8_t)(registers[A] << 1)) + c;
}

void CPU::rra() {
	uint8_t c = cy;
	cy = registers[A] & 1; 
    registers[A] = ((uint8_t)(registers[A] >> 1)) + ((uint8_t)(c << 7));
}

void CPU::daa() {
    uint8_t lsb = registers[A] & 0xf;
    uint8_t msb = registers[A] >> 4;
	uint8_t total = 0;
	bool c = cy; //cy backup

    if (hc || lsb > 9) {
    	total += 0x6;
	}
    if (cy || msb > 9 || (msb >= 9 && lsb > 9)) {
    	total += 0x60;
	}

    registers[A] = setCond(registers[A] + total, registers[A], total);
	cy = (c) ? c : cy;
}

void CPU::cma() {
	registers[A] = ~registers[A];
}

void CPU::stc() {
	cy = 1;
}

void CPU::cmc() {
	cy = !cy;
}

void CPU::ld8() {
	uint8_t reg = (opcode >> 3) & 0x7;
	if(reg == 0x6)
		changeM(memory[pc+1]);
	else 
		registers[reg] = memory[pc+1];
}

void CPU::mov() {
	uint8_t dst = (opcode >> 3) & 0x7;
	uint8_t src = opcode & 0x7;

	if(dst == 0x6)
		changeM(registers[src]);
	else if(src == 0x6)
		registers[dst] = memory[loc];
	else
		registers[dst] = registers[src];
}

void CPU::hlt() {
	hltB = true;
}

void CPU::math() {
	uint8_t data = registers[opcode & 0x7];

	if((opcode & 0xc0) == 0xc0)
		data = memory[pc+1];
	else if((opcode & 0x7) == 0x6)
		data = memory[loc];

	switch((opcode >> 3) & 0x7) {
		case 0x7:	setCond(registers[A] - data, registers[A], (~data + 1)); break;
		case 0x0:	registers[A] = setCond(registers[A] + data, registers[A], data); break;
		case 0x1:{ 	uint8_t actmp = ((registers[A] ^ data ^ cy ^ ((uint8_t)registers[A] + data + cy)) & 0x10) == 0x10; 
					registers[A] = setCond(registers[A] + (data + cy), registers[A], (data + cy));
					hc = actmp;
		} break;
		case 0x2:	registers[A] = setCond(registers[A] + (~data + 1), registers[A], (~data + 1));
					cy = !cy;
		break;
		case 0x3:{ 	uint8_t actmp = ((registers[A] ^ data ^ cy ^ ((uint8_t)registers[A] - data - cy)) & 0x10) == 0x10;
					registers[A] = setCond(registers[A] + (~(data + cy) + 1), registers[A], (~(data + cy) + 1));
					hc = actmp;
					cy = !cy;
		} break;
		case 0x4:{  uint8_t tmp = setCond(registers[A] & data, 0, 0);
				  	hc = ((registers[A] | data) & 0x8) != 0;
				  	cy = 0;
					registers[A] = tmp;
		} break;
		case 0x5:	registers[A] = setCond(registers[A] ^ data, 0, 0);
				  	n = hc = cy = 0;
		break;	
		case 0x6: 	registers[A] = setCond(registers[A] | data, 0, 0);
				  	hc = cy = 0;
		break; 
	}
}

void CPU::pop() { //11 rp(2) 0001
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) {
		uint8_t flags = memory[sp];
		z  = ((flags & 0x80) != 0);
		n  = ((flags & 0x40) != 0);
		hc = ((flags & 0x20) != 0);
		cy = ((flags & 0x10) != 0);
		registers[A] = memory[sp+1];
	} else {
		registers[reg+1] = memory[sp];
		registers[reg] = memory[sp+1];
	}

	sp += 2;
}

void CPU::push() { //11 rp(2) 0101
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) { //AF
        memory[sp-2] = (uint8_t)(z << 7) + (uint8_t)(n << 6) + (uint8_t)(hc << 5) + (uint8_t)(cy << 4);
		memory[sp-1] = registers[A];
	} else {
		memory[sp-2] = registers[reg+1];
		memory[sp-1] = registers[reg];
	}

	sp -= 2;
}

void CPU::ret() {
    pc = ((uint16_t)(memory[sp+1] << 8)) + memory[sp];
	sp += 2;
	cycles += 6;
}

void CPU::jmp() {
    pc = ((uint16_t)(memory[pc+2] << 8)) + memory[pc+1];
}

void CPU::call() {
	memory[sp-2] = (pc + 3) & 0xff;
	memory[sp-1] = (pc + 3) >> 8;
	sp -= 2;
    pc = ((uint16_t)(memory[pc+2] << 8)) + memory[pc+1];
	cycles += 6;
}

void CPU::retc() { //11 cond(3) 000
	if(checkCond())
		ret();
	else
		pc += 1;
}

void CPU::jmpc() { //11 cond(3) 010
	if(checkCond()) 
		jmp();
	else
		pc += 3;
}

void CPU::callc() { //11 cond(3) 100
	if(checkCond())
		call();
	else
		pc += 3;
}

void CPU::rst() { //11 exp(3) 111
	call();
	pc = (opcode & 0x38);

	hltB = false;
	interruptB = false;
}

void CPU::pchl() {
	pc = loc;
}

void CPU::sphl() {
	sp = loc;
}

void CPU::di() {
	interruptB = false;
}

void CPU::ei() {
	interruptB = true;
}

void CPU::stsp() {

}

void CPU::stop() {

}

void CPU::jr() {

}

void CPU::cpl() {

}

void CPU::scf() {

}

void CPU::ccf() {

}

void CPU::cb() {

}

void CPU::reti() {

}

void CPU::ldio() {

}

void CPU::tbl() {

}

void CPU::changeM(uint8_t value) {
    //if(loc >= vramStart && loc < (vramStart + 0x1C00)) {} was swapped out for a PPU SoC
	memory[loc] = value;
} 

bool CPU::checkCond() {
	uint8_t check = (opcode >> 3) & 0x7;

	return ((check == 0x0 && !z) || (check == 0x1 && z) || (check == 0x2 && !cy) || (check == 0x3 && cy));
}

uint8_t CPU::setCond(uint16_t ans, uint8_t old, uint8_t diff) {
	hc = ((old ^ diff ^ ans) & 0x10) == 0x10;
    cy = (ans > 0xff);
	z = (ans & 0xff) == 0;
    n = parity(ans & 0xff);

	return (ans & 0xff);
}

bool CPU::parity(uint16_t ans) {
    int count = 0;

    for (int i = 0; i < 8; i++)
        ((ans & (1 << i)) != 0) ? count++ : 0;

    return (count % 2 == 0); //need to fix later x d
}
