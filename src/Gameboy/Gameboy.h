#pragma once

#include "Memory.h"
#include "CPU.h"
#include "PPU.h"

#define NOMINMAX //for fmt/printf.h
#include <Windows.h>
#include <algorithm> //std::fill
#include <iterator> //std::end
#include <fstream> //std::istream
#include <set>
#include <string>

#include <chrono>
#include <cmath>
using namespace std::chrono;

constexpr auto CLOCK = 1048576; //m-cycle timings, aka for the lazy

struct Gameboy {
	Memory mem;

	//"Gameboy" stuff
	CPU cpu;
	PPU ppu;
	IMBC* mbc;
	std::ofstream log;
	u8 bios[0x100];
	bool IME = false;
	
	//Local Variables
	u64 uistep = 1;
	u64 step = 0;
	bool continue_trigger = true;
	bool step_trigger = false;
	bool stop_trigger = true;
	bool displayReady = true;
	std::mutex emustart;
	std::set<u16> breakpoints;

	Gameboy() : mem(*this), cpu(*this), ppu(*this) {
		std::fill(bios, std::end(bios), 0);
		mbc = new MBC0; //change this based on byte
	}

	~Gameboy() {
		delete mbc;
	}

	void LoadRom(std::istream& file) {
		std::ifstream f_bios("D:/dmg_boot.bin", std::ifstream::binary);
		f_bios.read((char*)bios, 0x100);
		f_bios.close();

		mbc->Setup(file);
					
		log.open("log.txt");
	}
	
	void SetBreakpoint(u16 PC) {
		breakpoints.insert(PC);
	}

	void Start() {
		emustart.lock();

		auto resetStep = [this]() {
			step_trigger = false;
			step = 0;
		};

		//I really should wrap this in it's own function and have a "Update" function based on a frame
		while(stop_trigger) {
			while(continue_trigger || step_trigger && step) {
				u8 cycles = cpu.ExecuteOpcode();

				ppu.update(cycles * 4);

				if(mem.IORegs[0x02] == 0xFF) {
					printf("%c", mem.IORegs[0x01]);
					mem.Write(0xFF02, u8(0x00));
				}

				if(step_trigger)
					step--;

				if(std::find(breakpoints.begin(), breakpoints.end(), cpu.PC) != breakpoints.end()) {
					continue_trigger = false;
					resetStep();
				}
			}

			if(!continue_trigger) { 
				if(step_trigger && step == 0) {
					resetStep();
				}
			}
		}

		printf("----------------------------------------------------\n");
	
		mem.Clean();
		cpu.Clean();
		log.close();

		emustart.unlock();
	}
};