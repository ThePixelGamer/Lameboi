#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class CPU {
public:
    vector<uint8_t> memory;
    bool            cy, hc, z, n, interruptB, hltB, soundB, runB;
    uint8_t         opcode, registers[8], bios[256];
    uint16_t        pc, sp, loc, cycles, cycBefore;

    CPU();
    void            emulateOpcode();
    bool            checkCond();
    uint8_t         setCond(uint16_t, uint8_t, uint8_t);
    bool            parity(uint16_t);
    void            changeM(uint8_t);

    void            nop();  //0x00 - 0x3f
    void            ld16();
    void            lda();
    void            incx();
    void            inc();
    void            dec();
    void            ld8();
    void            rlca();
    void            stsp();
    void            dad();
    void            decx();
    void            rrca();
    void            stop();
    void            rla();
    void            jr();
    void            rra();
    void            daa();
    void            cpl();
    void            scf();
    void            ccf();
    void            cma();
    void            sta();
    void            stc();
    void            cmc();        
    void            mov();  //0x40 - 0x7f
    void            hlt(); 
    void            math(); //0x80 - 0xbf  
    void            retc(); //0xc0 - 0xff
    void            pop();
    void            jmpc();
    void            jmp();        
    void            callc();
    void            push();
    void            rst();
    void            ret();
    void            cb();
    void            call();
    void            reti();
    void            ldio();
    void            tbl();
    void            di();
    void            pchl();
    void            sphl();
    void            ei();
    
    static constexpr void (CPU::*opcodeTable[256])() = {
    //          0            1          2            3          4            5           6            7           8          9           a            b          c            d           e            f
        &CPU::nop,  &CPU::ld16, &CPU::lda,  &CPU::incx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::rlca, &CPU::stsp, &CPU::dad,  &CPU::lda,  &CPU::decx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::rrca, //0
        &CPU::stop, &CPU::ld16, &CPU::lda,  &CPU::incx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::rla,  &CPU::jr,   &CPU::dad,  &CPU::lda,  &CPU::decx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::rra,  //1
        &CPU::jr,   &CPU::ld16, &CPU::lda,  &CPU::incx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::daa,  &CPU::jr,   &CPU::dad,  &CPU::lda,  &CPU::decx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::cpl,  //2
        &CPU::jr,   &CPU::ld16, &CPU::lda,  &CPU::incx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::scf,  &CPU::jr,   &CPU::dad,  &CPU::lda,  &CPU::decx, &CPU::inc,   &CPU::dec,  &CPU::ld8,  &CPU::ccf,  //3
        &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  //4
        &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  //5
        &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  //6
        &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::hlt,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,  &CPU::mov,   &CPU::mov,  &CPU::mov,  &CPU::mov,  //7
        &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, //8
        &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, //9
        &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, //a
        &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math, &CPU::math,  &CPU::math, &CPU::math, &CPU::math, //b
        &CPU::retc, &CPU::pop,  &CPU::jmpc, &CPU::jmp,  &CPU::callc, &CPU::push, &CPU::math, &CPU::rst,  &CPU::retc, &CPU::ret,  &CPU::jmpc, &CPU::cb,   &CPU::callc, &CPU::call, &CPU::math, &CPU::rst,  //c
        &CPU::retc, &CPU::pop,  &CPU::jmpc, &CPU::nop,  &CPU::callc, &CPU::push, &CPU::math, &CPU::rst,  &CPU::retc, &CPU::reti, &CPU::jmpc, &CPU::nop,  &CPU::callc, &CPU::nop,  &CPU::math, &CPU::rst,  //d
        &CPU::ldio, &CPU::pop,  &CPU::ldio, &CPU::nop,  &CPU::nop,   &CPU::push, &CPU::math, &CPU::rst,  &CPU::tbl,  &CPU::pchl, &CPU::sta,  &CPU::nop,  &CPU::nop,   &CPU::nop,  &CPU::math, &CPU::rst,  //e
        &CPU::ldio, &CPU::pop,  &CPU::ldio, &CPU::di,   &CPU::nop,   &CPU::push, &CPU::math, &CPU::rst,  &CPU::tbl,  &CPU::sphl, &CPU::lda,  &CPU::ei,   &CPU::nop,   &CPU::nop,  &CPU::math, &CPU::rst   //f
    }; 
};
