#include "Memory.h"

#include "Gameboy.h"

Memory::Memory(Gameboy& t_gb) : gb(t_gb) {
	clean();

	// initialize memorytags
}

void Memory::clean() {
	WRAM.fill(0);
	HRAM.fill(0);

	dma = DMA_START = 0;
	boot = true;
}

void Memory::update() {
	if (dma == 0) return;

	// DMA Transfer, 1 byte per M Cycle
	if (dma <= DMA_SIZE) {
		u8 idx = (DMA_SIZE - dma);
		u16 dmaAddr = (DMA_START << 8) | idx;
		gb.ppu.writeOAM(idx, read(dmaAddr), true);
	}
	--dma;
}

enum MemTag : u8 {
	BIOS,
	ROM0,
	ROM1,
	VRAM,
	ERAM,
	WRAM, // todo: split into 2 for CGB support
	ECHO,
	OAM, // OAM/Unsupported
	HIGH, // I/O, HRAM, IE
};

constexpr auto memtags = []() {
	std::array<MemTag, 256> tmp{};
	tmp[0] = BIOS; // 0x0000-0x0100

	int i = 0x1;
	while (i < 0x40) tmp[i++] = ROM0; // 0x0100-0x3FFF
	while (i < 0x80) tmp[i++] = ROM1; // 0x4000-0x7FFF
	while (i < 0xA0) tmp[i++] = VRAM; // 0x8000-0x9FFF
	while (i < 0xC0) tmp[i++] = ERAM; // 0xA000-0xBFFF
	while (i < 0xE0) tmp[i++] = WRAM; // 0xC000-0xDFFF
	while (i < 0xFE) tmp[i++] = ECHO; // 0xE000-0xFDFF
	tmp[i++] = OAM; // 0xFE00-0xFEFF
	tmp[i] = HIGH; // 0xFF00-0xFFFF

	return tmp;
}();

// todo: template for debugger?
u8 Memory::cpu_read(addr loc) {
	MemTag tag = memtags[loc >> 8];
	const u8 low = loc & 0xFF;

	// DMA Blocking 
	if (dma != 0) {
		if (tag == HIGH && (low & 0x80) && low < 0xFF) {
			return HRAM[loc & 0x7F];
		}

		// todo: handle OAM Corruption

		return 0xFF;
	}

	return read(loc);
}

u8 Memory::read(addr loc) {
	MemTag tag = memtags[loc >> 8];
	const u8 low = loc & 0xFF;

	switch (tag) {
		case HIGH: return read_high(low);

		case ECHO:
		case MemTag::WRAM: return WRAM[loc & 0x1FFF];

		case BIOS:
			if (boot) {
				return gb.bios[low];
			}
			[[fallthrough]];
		
		case ROM0: return gb.cart.readBank0(loc & 0x3FFF);
		case ROM1: return gb.cart.readBank1(loc & 0x3FFF);
		case ERAM: return gb.cart.readRam(loc & 0x1FFF);

		case VRAM: return gb.ppu.readVRAM(loc & 0x1FFF);
		case OAM: return (low < 0xA0) ? gb.ppu.readOAM(low) : 0x0;

		default: return 0xFF;
	}
}

void Memory::cpu_write(u16 loc, u8 value) {
	// DMA Blocking 
	if (dma != 0) {
		// todo: handle OAM Corruption
		return;
	}

	MemTag tag = memtags[loc >> 8];
	const u8 low = loc & 0xFF;

	switch (tag) {
		case HIGH: write_high(low, value); break;

		case ECHO:
		case MemTag::WRAM: WRAM[loc & 0x1FFF] = value; break;

		case BIOS:
		case ROM0: gb.cart.writeBank0(loc & 0x3FFF, value); break;
		case ROM1: gb.cart.writeBank1(loc & 0x3FFF, value); break;
		case ERAM: gb.cart.writeRam(loc & 0x1FFF, value); break;

		case VRAM: gb.ppu.writeVRAM(loc & 0x1FFF, value); break;
		case OAM: if (low < 0xA0) { gb.ppu.writeOAM(low, value); } break;

		default: break;
	}
}

u8 Memory::read_high(u8 loc) {
	u8 reg = loc & 0x7F;

	if (loc & 0x80) {
		if (loc == IO::IE) 
			return gb.interrupt.enable.read();
		else
			return HRAM[reg];
	}
	else {
		return read_io(reg);
	}
}

void Memory::write_high(u8 loc, u8 value) {
	u8 reg = loc & 0x7F;

	if (loc & 0x80) {
		if (loc == IO::IE)
			gb.interrupt.enable.write(value);
		else
			HRAM[loc & 0x7F] = value;
	}
	else {
		write_io(reg, value);
	}
}

namespace IO {

enum IOTag : u8 {
	Unused,
	Joypad,
	Serial,
	Timer,
	Interrupt,
	Audio,
	Wave,
	PPU,
	DMA,
	BOOT,
};

constexpr auto tags = []() {
	std::array<IOTag, 128> tmp{};

	tmp[0x00] = Joypad; // 0xFF00
	for (int i = 0x01; i < 0x03;) tmp[i++] = Serial; // 0xFF01-0xFF02
	for (int i = 0x04; i < 0x08;) tmp[i++] = Timer; // 0xFF04-0xFF07
	tmp[0x0F] = Interrupt; // 0xFF0F
	for (int i = 0x10; i < 0x27;) tmp[i++] = Audio; // 0xFF10-0xFF26
	for (int i = 0x30; i < 0x40;) tmp[i++] = Wave; // 0xFF30-0xFF3F
	for (int i = 0x40; i < 0x4C;) tmp[i++] = PPU; // 0xFF40-0xFF4B
	tmp[0x46] = DMA; // 0xFF46
	tmp[0x50] = BOOT; // 0xFF50

	return tmp;
}();

}

u8 Memory::read_io(u8 reg) {
	auto iotag = IO::tags[reg];

	switch (iotag) {
		case IO::Joypad: return gb.joypad.read();
		case IO::Serial: return gb.serial.read(reg);
		case IO::Timer: return gb.timer.read(reg);
		case IO::Audio: return gb.apu.read_reg(reg);
		case IO::Wave: return gb.apu.read_wave(reg);
		case IO::PPU: return gb.ppu.read(reg);

		case IO::Interrupt: return gb.interrupt.request.read();
		case IO::DMA: return DMA_START;
		case IO::BOOT: return boot;

		case IO::Unused: 
		default: return 0xFF;
	}
}

void Memory::write_io(u8 reg, u8 value) {
	auto iotag = IO::tags[reg];

	switch (iotag) {
		case IO::Joypad: gb.joypad.write(value); break;
		case IO::Serial: gb.serial.write(reg, value); break;
		case IO::Timer: gb.timer.write(reg, value); break;
		case IO::Audio: gb.apu.write_reg(reg, value); break;
		case IO::Wave: gb.apu.write_wave(reg, value); break;
		case IO::PPU: gb.ppu.write(reg, value); break;

		case IO::Interrupt: gb.interrupt.request.write(value); break;
		case IO::DMA: {
			if (dma == 0) {
				DMA_START = value;
				dma = DMA_SIZE;
			}
		} break;
		case IO::BOOT: boot = !value; break;

		case IO::Unused: break;
	}
}