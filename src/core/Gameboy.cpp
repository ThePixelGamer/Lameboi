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

	if (fs::file_size(biosPath) != bios.size()) {
		LB_WARN(Frontend, "{} file is not a valid DMG bios", biosPath);
		return false;
	}

	std::ifstream biosFile(biosPath, std::ifstream::binary);

	if (!biosFile) {
		LB_WARN(Frontend, "{} file failed to open", biosPath);
		return false;
	}

	biosFile.read((char*)bios.data(), bios.size());
	biosFile.close();
	return true;
}

bool Gameboy::loadRom(const std::string& romPath, bool start, bool power) {
	if (!cart.load(romPath)) {
		return false;
	}

	spriteManager.loadRom(cart.romName);

	if (power) {
		if (!start) debug.pause();
		this->start();
	}

	return true;
}

void Gameboy::run() {
	while (emuRun) {
		if (debug.shouldBreak(cpu.PC)) {
			continue;
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

void Gameboy::clean() {
	// save any battery backed components to a file
	cart.unload();

	mem.clean();
	cpu.clean();
	ppu.clean();
	apu.clean();
	interrupt.clean();
	joypad.clean();
	timer.clean();
	serial.clean();
}