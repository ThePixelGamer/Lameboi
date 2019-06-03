#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <functional>

typedef uint8_t* puint8;
typedef uint16_t* puint16;

using namespace std;

class CPU {
public:
    vector<uint8_t> memory;
    bool            interruptB, hltB, soundB, runB;
    uint8_t         opcode, bios[256];
    puint8          A, F, B, C, D, E, H, L;
    uint16_t        PC, cycles, cycBefore;
    puint16         SP = new uint16_t(0), AF = new uint16_t(0), BC = new uint16_t(0), DE = new uint16_t(0), HL = new uint16_t(0);

    CPU();
    void            emulateOpcode();
    bool            checkFlags(uint8_t);
    uint8_t         setFlags(uint16_t, uint8_t, uint8_t, uint8_t);
    bool            parity(uint16_t);
    void            changeM(uint8_t);
    uint8_t         getNextOffsetByte(int);
    uint16_t        getNext2Bytes();

    void            nop();  //0x00 - 0x3f
    void            ld8(uint8_t*, uint8_t);
    void            math8(uint8_t*, uint8_t, function<uint16_t(uint8_t, uint8_t)>, uint8_t);
    void            ld16(uint16_t*, uint16_t);
    void            add16();
    void            inc16();
    void            dec16();
    void            daa();
    void            scf();
    void            cpl();
    void            ccf();
    void            halt();
    void            ei();
    void            di();
    void            call();
    void            jp();
    void            jr(uint8_t, int8_t, bool);
    void            ret();
    void            rst();
    void            push(uint16_t);
    uint16_t        pop();

    void            bit(int, uint8_t);

};
