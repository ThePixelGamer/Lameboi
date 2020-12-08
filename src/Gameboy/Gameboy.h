#pragma once

#include "Memory.h"
#include "MBC.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Interrupt.h"
#include "Scheduler.h"
#include "Debugger.h"

#include "IO.h"
#include "io/Joypad.h"
#include "io/Timer.h"
#include "io/SerialPort.h"

#include <algorithm> //std::fill
#include <chrono>
#include <cmath>
#include <iterator> //std::end
#include <fstream> //std::istream
#include <memory>
#include <set>
#include <string>
#include <fmt/printf.h>

struct Gameboy {
	Memory mem;
	std::array<u8, 0x100> bios;
	std::unique_ptr<IMBC> mbc;

	CPU cpu;
	PPU ppu;
	APU apu;
	Interrupt interrupt;
	IO io;
	Joypad joypad;
	Timer timer;
	SerialPort serial;

	Scheduler scheduler;

	Debugger debug;
	std::ofstream log;

	//Local Variables
	bool running = false;
	bool finished = true;
	std::condition_variable emustart;
	std::mutex emustart_m;

	Gameboy() :
		mem(*this),
		scheduler(*this),
		interrupt(),
		cpu(mem, scheduler, interrupt),
		ppu(interrupt),
		apu(),
		io(*this),
		joypad(interrupt),
		timer(interrupt),
		serial(),
		debug(mem) {
		bios.fill(0xFF);
		mbc = nullptr;
	}

	bool LoadRom(std::istream& file) {
		// doing this because it looks better in mem editor /shrug
		std::ifstream f_bios("D:/dmg_boot.bin", std::ifstream::binary);
		f_bios.read(reinterpret_cast<char*>(bios.data()), 0x100);
		f_bios.close();

		u8 type = file.seekg(0x147).get(); //get mbc type
		file.seekg(0); //reset ifstream position
		
		mbc = loadMBCFromByte(type);
		if (mbc == nullptr) {
			bios.fill(0xFF);
			return false;
		}

		mbc->setup(file);
		log.open("log.txt");
		return true;
	}

	void Start() {
		finished = false;
		running = true;

		while(running) {
			for (size_t steps = debug.amountToStep(cpu.PC); steps > 0; --steps) {
				if (!running) {
					break;
				}
				
				cpu.ExecuteOpcode();
				
				/*
				if (mem.serialControl.transferStart) {
					std::cout << mem.serialData;
					mem.serialControl.transferStart = 0;
				}
				*/
			}
		}

		printf("----------------------------------------------------\n");
		
		{
			std::unique_lock lock(emustart_m);
			finished = true;
			emustart.notify_one();
		}

		// write any ram to a file if the mbc needs to
		if (mbc) {
			mbc->close();
		}

		bios.fill(0xFF);

		mem.clean();
		cpu.clean();
		ppu.clean();
		apu.clean();
		log.close();
		timer.clean();
	}
};