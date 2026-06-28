#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
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

#include "io/Joypad.h"
#include "io/Timer.h"
#include "io/SerialPort.h"

#include "util/Log.h"
#include "util/FileUtil.h"

class Gameboy {
private:
	// Threading
	std::mutex m;
	std::condition_variable cv;
	std::atomic_bool threadRun = true;

public:
	std::atomic_bool emuRun = false;

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

	// I/O
	Joypad joypad;
	Timer timer;
	SerialPort serial;

	Debugger debug;

	Gameboy() :
		mem(*this),
		scheduler(*this),
		interrupt(),
		spriteManager(ppu, mem.boot),
		cpu(mem, scheduler, interrupt),
		ppu(*this),
		apu(),
		joypad(interrupt),
		timer(interrupt),
		serial(),
		debug(mem) {
		bios.fill(0xFF);

		createDirectory("saves");
	}

	bool loadBios(const std::string& biosPath);
	bool loadRom(const std::string& romPath, bool start = true, bool power = true);

	// signal run thread to start executing 
	void start() {
		emuRun = true;
		std::unique_lock lk(m);
		cv.notify_one();
	}

	// wait for thread to stop executing
	void stop() {
		// stop only if running 
		if (emuRun) {
			emuRun = false;
			std::unique_lock lk(m);
			cv.wait(lk);
		}
	}

	// signal and wait for thread to finish 
	void exit() {
		threadRun = false;
		
		// Core never started so "start" to cleanly exit
		if (!emuRun) {
			std::unique_lock lk(m);
			cv.notify_one();
		}

		emuRun = false;
		std::unique_lock lk(m);
		cv.wait(lk);
	}

	// the main thread function
	void thread() {
		while (threadRun) {
			{
				std::unique_lock lk(m);
				cv.wait(lk);
			}

			run();
			clean();

			// notify stop()/exit() that we finished cleaning up
			std::unique_lock lk(m);
			cv.notify_one();
		}
	}

private:
	void run();
	void clean();
};