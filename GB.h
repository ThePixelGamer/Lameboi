#pragma once

#include <string>

#include "CPU.h"
#include "PPU.h"

using namespace std;

class GB {
public:
    CPU* cpu;
    PPU* ppu;

    GB();
    void load(string);
};
