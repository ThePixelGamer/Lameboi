#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <functional>

typedef uint8_t u8;
typedef uint16_t u16;
typedef u8* pu8;

using namespace std;

class CPU {
public:
    vector<u8>  memory;
    bool        interruptB, hltB, soundB, runB;
    u8          opcode, bios[256];
    pu8         A, F, B, C, D, E, H, L;
    u16         PC, cycles, cycBefore, SP, AF, BC, DE, HL;

    CPU();
    void        emulateOpcode();
    bool        checkFlags(u8);
    u8          setFlags(u16, u8, u8, u8);
    bool        parity(u16);
    void        changeM(u8);
    u8          getNextOffsetByte(int);
    u16         getNext2Bytes();

    void        nop();  //0x00 - 0x3f
    void        ld8(pu8, u8);
    void        math8(pu8, u8, function<u16(u8, u8)>, u8);
    void        ld16(u16&, u16);
    void        add16();
    void        inc16();
    void        dec16();
    void        daa();
    void        scf();
    void        cpl();
    void        ccf();
    void        halt();
    void        ei();
    void        di();
    void        call();
    void        jp();
    void        jr(u8, char, bool);
    void        ret();
    void        rst();
    void        push(u16);
    u16         pop();

    void        bit(int, u8);

};
