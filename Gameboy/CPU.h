#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <functional>
#include <cstdint>
#include <stdexcept>
#include <variant>

#include "Memory.h"

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

using std::string;
using std::vector;

union rp {
  struct {
    u8 low;
    u8 high;
  };
  u16 value;
};

struct CPU {
    rp af{};
    rp bc{};
    rp de{};
    rp hl{};

    u8& A = af.high;
    u8& F = af.low;
    u8& B = bc.high;
    u8& C = bc.low;
    u8& D = de.high;
    u8& E = de.low;
    u8& H = hl.high;
    u8& L = hl.low;

    u16& AF = af.value;
    u16& BC = bc.value;
    u16& DE = de.value;
    u16& HL = hl.value;

    vector<u8>  memory;
    bool        IME, soundB, runB;
    u8          opcode, low_power;
    u8*         M;
    u16         PC, SP, cycles, cycBefore;
    FILE        *log;

    CPU();
    void        emulateOpcode();
    bool        checkFlags(u8);
    u16         setFlags(u16, u8, u8, u8);
    bool        parity(u16);
    u16         getNextBytes(int);
    void        handlePrint();

    template    <typename T>
    void        math(T&, u8, std::function<u16(u16, u8)>, u8, bool = false, bool = false);
    void        add16(u16&, u16);
    void        inc16(u16&);
    void        dec16(u16&);
    void        ld8(u8&, u8);
    void        ld16(u16&, u16);
    void        daa();
    void        scf();
    void        cpl();
    void        ccf();
    void        halt();
    void        ei();
    void        di();
    void        call(u8 = 0, bool = true);
    void        jp(u8 = 0, bool = true);
    void        jr(u8 = 0, bool = true);
    void        ret(u8 = 0, bool = true);
    void        rst();
    void        push(u16);
    u16         pop();

    //cb
    void        bit(int, u8);
};
