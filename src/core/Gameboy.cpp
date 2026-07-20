#include "Gameboy.h"

#include <filesystem>
#include <iostream>

#include <fmt/printf.h>

namespace fs = std::filesystem;

#define OUTPUT_CPU false

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
	std::ofstream bootlog("bootrom.txt");
	std::ofstream romlog("rom.txt");

	while (emuRun) {
		if (debug.shouldBreak(cpu.PC)) {
			continue;
		}

		// calls step() every m cycle within each instruction
		cpu.update();

		if (OUTPUT_CPU) {
			auto& log = (mem.boot) ? bootlog : romlog;
			log << fmt::format("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: {:02X}:{:04X} ({:02X} {:02X} {:02X} {:02X})\n",
				cpu.A, u8(cpu.F), cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L, cpu.SP, 0, cpu.PC - 1, cpu.IR, mem.cpu_read(cpu.PC), mem.cpu_read(cpu.PC + 1), mem.cpu_read(cpu.PC + 2));
		}
	}
}

void Gameboy::step() {
	mem.update();
	apu.update();
	ppu.update();
	timer.update();

	serial.print();
}

void Gameboy::clean() {
	// save any battery backed components to a file
	cart.unload();

	mem.clean();
	cpu.reset();
	ppu.clean();
	apu.clean();
	interrupt.clean();
	joypad.clean();
	timer.clean();
	serial.clean();
}
