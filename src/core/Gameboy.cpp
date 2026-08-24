#include "Gameboy.h"

#include "Config.h"

#include <filesystem>
#include <iostream>

#include <fmt/printf.h>

namespace fs = std::filesystem;

#define OUTPUT_CPU false

bool Gameboy::loadRom(const std::string& romPath, bool start, bool power) {
	if (!cart.load(romPath)) {
		return false;
	}

	auto model = (config.model == Model::AUTO) ? cart.getHeader()->getModel() : *config.model;

	cgbMode = model == Model::CGB;

	bus.install(model);
	cart.install();
	apu.install(bus);
	cpu.install(bus, model);
	ppu.install(bus, model);
	joypad.install(bus);
	serial.install(bus);
	timer.install(bus);
	interrupt.install(bus);

	// load bios
	[&]() {
		std::string biosPath = config.biosDir;
		if (config.fastBios) biosPath += "fast_";

		switch (model) {
			default:
			case Model::DMG: biosPath += "dmg_boot.bin"; break;
			case Model::CGB: biosPath += "cgb_boot.bin"; break;
		}

		if (!fs::exists(biosPath)) {
			auto fast = "fast_";
			auto idx = biosPath.rfind(fast);
			if (idx != biosPath.npos)
				biosPath.erase(idx, std::char_traits<char>::length(fast));
			
			if (!fs::exists(biosPath)) {
				LB_WARN(Frontend, "{} file does not exist", biosPath);
				return;
			}
		}

		cpu.bios.resize(fs::file_size(biosPath));
		std::ifstream biosFile(biosPath, std::ifstream::binary);

		if (!biosFile) {
			LB_WARN(Frontend, "{} file failed to open", biosPath);
			return;
		}

		biosFile.read((char*)cpu.bios.data(), cpu.bios.size());
		biosFile.close();
	}();

	// load cg manifest for game 
	ppu.game.load(cart.romName);

	if (power) {
		if (!start) debug.pause();
		this->start();
	}

	return true;
}

void Gameboy::run() {
	std::ofstream bootlog, romlog;
	if (OUTPUT_CPU) {
		bootlog.open("logs/bootrom.txt");
		romlog.open("logs/rom.txt");
	}

	while (emuRun) {
		if (debug.shouldBreak(cpu.PC)) {
			continue;
		}

		// calls step() every m cycle within each instruction
		cpu.update();

		if (OUTPUT_CPU) {
			auto& log = (cpu.inBios) ? bootlog : romlog;
			log << cpu.log();
		}
	}
}

void Gameboy::step(bool doubleSpeed) {
	apu.update(doubleSpeed);
	ppu.update(doubleSpeed);

	u8 count = 1 << doubleSpeed;
	while (count--) {
		timer.update();
		//serial.print();
	}
}

void Gameboy::clean() {
	// save any battery backed components to a file
	cart.unload();

	bus.clean();
	cpu.reset();
	ppu.clean();
	apu.clean();
	interrupt.clean();
	joypad.clean();
	timer.clean();
	serial.clean();
}
