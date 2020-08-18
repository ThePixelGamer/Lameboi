#pragma once

#include "Memory.h"
#include "CPU.h"
#include "PPU.h"
#include "Joypad.h"
#include "Scheduler.h"
#include "../Debug/Debugger.h"

#include <algorithm> //std::fill
#include <iterator> //std::end
#include <fstream> //std::istream
#include <set>
#include <string>

#include <chrono>
#include <cmath>

struct Gameboy {
	Memory mem;
	std::array<u8, 0x100> bios;
	std::unique_ptr<IMBC> mbc;

	CPU cpu;
	bool IME = false;

	PPU ppu;

	Joypad pad;

	Scheduler scheduler;

	Debugger debug;
	std::ofstream log;

	//Local Variables
	bool running = false;
	bool finished = true;
	std::condition_variable emustart;
	std::mutex emustart_m;

	Gameboy() : mem(*this), cpu(*this), ppu(*this), pad(mem), debug(mem), scheduler(*this) {
		bios.fill(0);
		mbc = std::make_unique<MBC0>(); //change this based on byte
	}

	void LoadRom(std::istream& file) {
		std::ifstream f_bios("D:/dmg_boot.bin", std::ifstream::binary);
		f_bios.read(reinterpret_cast<char*>(bios.data()), 0x100);
		f_bios.close();

		mbc->Setup(file);
					
		log.open("log.txt");
	}

	void Start() {
		finished = false;
		running = true;

		while(running) {
			for (size_t steps = debug.amountToStep(cpu.PC); steps > 0; --steps) {
				cpu.ExecuteOpcode();
			}
		}

		printf("----------------------------------------------------\n");
		
		{
			std::unique_lock lock(emustart_m);
			finished = true;
			emustart.notify_one();
		}

		mem.clean();
		cpu.clean();
		ppu.clean();
		log.close();
	}
};