#include "Gameboy.h"

#include <filesystem>

#include <fmt/printf.h>

namespace fs = std::filesystem;

// todo: verify the file provided is 0xFF bytes?
bool Gameboy::loadBios(const std::string& biosPath) {
	if (!fs::exists(biosPath)) {
		LB_WARN(Frontend, "{} file does not exist", biosPath);
		return false;
	}

	std::ifstream biosFile(biosPath, std::ifstream::binary);

	if (!biosFile) {
		LB_WARN(Frontend, "{} file failed to open", biosPath);
		return false;
	}

	biosFile.read(reinterpret_cast<char*>(bios.data()), 0x100);
	biosFile.close();
	return true;
}

bool Gameboy::loadRom(const std::string& romPath) {
	if (!fs::exists(romPath)) {
		LB_WARN(Frontend, "{} file does not exist", romPath);
		return false;
	}

	std::ifstream romFile(romPath, std::ifstream::binary);

	if (!romFile) {
		LB_WARN(Frontend, "{} file failed to open", romPath);
		return false;
	}

	u8 type = romFile.seekg(0x147).get(); //get mbc type
	romFile.seekg(0); //reset ifstream position

	romContext.fileName = fs::path(romPath).stem().string();
	mbc = MBC::createInstance(romContext.fileName, type);
	if (mbc == nullptr) {
		return false;
	}

	mbc->setup(romFile);
	romFile.close();
	log.open("log.txt");
	return true;
}

void Gameboy::start() {
	std::lock_guard<std::mutex> lk(emuM);
	emuRun = true;
	emuDone = false; 
	emuCV.notify_one();
}

void Gameboy::stop() {
	if (!emuDone) {
		emuRun = false;
		std::unique_lock lk(emuM);
		emuCV.wait(lk, [this] { return emuDone; });
	}
}

void Gameboy::close() {
	std::unique_lock lk(emuM);
	emuRun = false;
	// hack to get the wait to work, need to rework in context of threadRun
	emuCV.wait(lk, [this] { return emuDone; });
}

// https://en.cppreference.com/w/cpp/thread/condition_variable
void Gameboy::run() {
	while (threadRun) {
		std::unique_lock lk(emuM);
		emuCV.wait(lk, [this] { return !emuDone; });

		while (emuRun) {
			for (size_t steps = debug.amountToStep(cpu.PC); emuRun && (steps > 0); --steps) {
				cpu.ExecuteOpcode();

				/*
				if (mem.serialControl.transferStart) {
					std::cout << mem.serialData;
					mem.serialControl.transferStart = 0;
				}
				*/
			}
		}

		// todo: should this print be changed?
		if (threadRun) {
			fmt::printf("----------------------------------------------------\n");
		}

		clean();

		// loading bios once now
		//bios.fill(0xFF);

		// not needed, remove?
		if (log.is_open()) {
			log.close();
		}

		emuDone = true;
		lk.unlock();
		emuCV.notify_one();
	}
}

void Gameboy::clean() {
	// write any ram to a file if the mbc needs to
	if (mbc) {
		mbc->close();
	}

	mem.clean();
	cpu.clean();
	ppu.clean();
	apu.clean();
	interrupt.clean();
	joypad.clean();
	timer.clean();
	serial.clean();
}