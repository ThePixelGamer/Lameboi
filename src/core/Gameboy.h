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
#include "util/FileUtil.h"

class Gameboy {
public:
	// Internal
	Memory mem;
	std::array<u8, 0x100> bios;
	Cartridge cart;
	
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
		spriteManager(ppu, mem.boot),
		cpu(mem, scheduler, interrupt),
		ppu(*this),
		apu(),
		io(*this),
		joypad(interrupt),
		timer(interrupt),
		serial(),
		debug(mem) {
		bios.fill(0xFF);

		createDirectory("saves");
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