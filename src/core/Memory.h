#pragma once

#include <array>

#include <util/Types.h>

class Gameboy;

// https://gbdev.io/pandocs/Memory_Map.html
class Memory {
private:
	Gameboy& gb;

	//regs
	std::array<u8, 0x2000> WRAM; //0xC000-0xDFFF
	std::array<u8, 0x7F> HRAM; //0xFF80

	constexpr static u8 DMA_SIZE = 0xA0;
	u8 DMA_START; //0xFF46 DMA Transfer and Start Address
	u8 dma = 0; //which byte we're currently copying

public:
	bool boot; //0xFF50, need to move this

	Memory(Gameboy&);
	void clean();
	void update(); //oam dma

	u8 cpu_read(addr loc);
	void cpu_write(addr loc, u8 value);

	u8 read_high(u8 loc);
	void write_high(u8 loc, u8 value);

private:
	u8 read(u16 loc);

	u8 read_io(u8 reg);
	void write_io(u8 reg, u8 value);
};