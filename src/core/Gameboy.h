#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

#include "Memory.h"
#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Interrupt.h"
#include "Debugger.h"

#include "io/Joypad.h"
#include "io/Timer.h"
#include "io/SerialPort.h"

#include "util/FileUtil.h"

class Gameboy {
private:
	// Threading
	std::mutex m;
	std::condition_variable cv;
	std::atomic_bool threadRun = true;
	std::thread emuThread;

public:
	std::atomic_bool emuRun = false;

	// Internal
	Debugger debug;
	Memory bus;
	Cartridge cart;

	CPU cpu;
	PPU ppu;
	APU apu;

	// I/O
	Interrupt interrupt;
	Joypad joypad;
	Timer timer;
	SerialPort serial;

	Gameboy() :
		bus(*this),
		cart(bus),
		interrupt(),
		cpu(*this),
		ppu(*this),
		apu(),
		joypad(interrupt),
		timer(interrupt),
		serial(),
		debug(bus) {
		createDirectory("saves");
		emuThread = std::thread(&Gameboy::thread, this);
	}

	~Gameboy() {
		exit();
		emuThread.join();
	}

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
		cpu.quit = true;
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

	void step();

private:
	void run();
	void clean();
};
