#pragma once

#include "Memory.h"
#include "CPU.h"
#include "PPU.h"

#define NOMINMAX //for fmt/printf.h
#include <Windows.h>
#include <string>
#include <algorithm> //std::fill
#include <iterator> //std::end
#include <fstream> //std::istream

struct Gameboy {
	Memory mem;

	//"Gameboy" stuff
	CPU cpu;
	PPU ppu;
	IMBC* mbc;
	FILE* log;
	u8 bios[0x100];
	bool IME;
	
	//Local Variables
	u64 step_old = 1;
	u64 step = 1;
	bool continue_trigger = true;
	bool step_trigger = false;
	bool stop_trigger = true;
	std::mutex emustart;

	Gameboy() : mem(*this), cpu(*this), ppu(*this) {
		std::fill(bios, std::end(bios), 0);
		mbc = new MBC0; //change this based on byte
	}

	~Gameboy() {
		delete mbc;
	}

	void LoadRom(std::istream& file) {
		std::ifstream f_bios("dmg_boot.bin", std::ifstream::binary);
		f_bios.read((char*)bios, 0x100);
		f_bios.close();

		mbc->Setup(file);
					
		log = fopen("log.txt", "w");
	}

	void Start() {
		emustart.lock();
		while(stop_trigger) {
			while(continue_trigger || (step_trigger && step)) {
				try {
					cpu.ExecuteOpcode();
				}
				catch(const char* msg) {
					std::cerr << msg << std::endl;
				}

				if(step_trigger)
					step--;

				if (mem.IORegs[0x02] == 0xFF) {
					char c = mem.IORegs[0x01];
					printf("%c", c);
					mem.Write(0xFF02, u8(0x00));
				}
			}

			if(!continue_trigger) { 
				if(step_trigger && step == 0) {
					step_trigger = false;
					step = step_old;
				}
			}
		}

		printf("----------------------------------------------------\n");
	
		mem.Clean();
		cpu.Clean();
		fclose(log);

		emustart.unlock();
	}
};