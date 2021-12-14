#pragma once

#include <algorithm> //std::fill
#include <chrono>
#include <cmath>
#include <fstream> //std::istream
#include <memory>
#include <set>
#include <string>

#include "Memory.h"
#include "mbc/MBC.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Interrupt.h"
#include "Scheduler.h"
#include "SpriteManager.h"
#include "Debugger.h"

#include "IO.h"
#include "io/Joypad.h"
#include "io/Timer.h"
#include "io/SerialPort.h"

#include "util/Log.h"

// todo: add more context?
struct RomContext {
	std::string fileName;
	std::string title; // 0x134 - 0x143, assuming not a cgb cartridge
	CartridgeType cartType; // 0x147
	u8 romSize; // 0x148, just uses the code
	u8 ramSize; // 0x149, just uses the code
	u16 checksum; // 0x14E-0x14F
};

class Gameboy {
public:
	// Internal
	Memory mem;
	std::array<u8, 0x100> bios;
	std::unique_ptr<MBC> mbc;

	Scheduler scheduler;
	Interrupt interrupt;
	SpriteManager spriteManager;

	CPU cpu;
	PPU ppu;
	APU apu;
	IO io;
	Joypad joypad;
	Timer timer;
	SerialPort serial;

	RomContext romContext;
	Debugger debug;
	std::ofstream log;

	// Threading
	std::mutex emuM;
	std::condition_variable emuCV;
	std::atomic_bool threadRun = true;
	std::atomic_bool emuRun = false;
	bool emuDone = true;

	Gameboy() :
		mem(*this),
		scheduler(*this),
		interrupt(),
		spriteManager(ppu, romContext, mem.boot),
		cpu(mem, scheduler, interrupt),
		ppu(interrupt, spriteManager),
		apu(),
		io(*this),
		joypad(interrupt),
		timer(interrupt),
		serial(),
		debug(mem) {
		bios.fill(0xFF);
		mbc = nullptr;
	}

	bool loadBios(const std::string& biosPath);
	bool loadRom(const std::string& romPath);

	// todo: better comments :/

	// rom is loaded, continue running in thread
	void start();

	// check to see if thread is running, "close" if so
	void stop();

	// ignore the check, "close" the thread
	void close();

	// the main thread function
	void run();

private:
	void clean();
};