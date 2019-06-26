#pragma once

#include <string>
#include <QApplication>
#include <QThread>
#include <QTime>

#include "emulatorscreen.h"
#include "CPU.h"
#include "PPU.h"
#include "Memory.h"

using namespace std;

class GB {
public:
    CPU* cpu;
    PPU* ppu;
    bool running = true;

    GB();
    void load(string, EmulatorScreen*);
};
