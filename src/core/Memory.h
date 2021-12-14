#pragma once

//https://gbdev.gg8.se/wiki/articles/Memory_Map

#include <array>
#include <iostream>
#include <mutex>

#include "util/Types.h"
#include "ppu/Palette.h"
#include "ppu/Sprite.h"

class Gameboy;

class Memory {
	Gameboy& gb;

	//regs
	std::array<u8, 0x2000> WRAM; //0xC000-0xDFFF
	u8 DMA; //0xFF46 DMA Transfer and Start Address

public:
	bool boot; //0xFF50, need to move this

private:
	std::array<u8, 0x60> undocumented; //0xFEA0
	std::array<u8, 0x7F> HRAM; //0xFF80

	//internal
	bool memoryRead = false; //bit of a hack to use the read function during dma
	u16 dmaAddr = 0; //what address the dma is currently on
	u8 dmaOffset = 0; //which byte we're currently copying

public:
	Memory(Gameboy&);
	void clean();
	void update(); //oam dma

	u8 read(u16 loc);
	void write(u16 loc, u8 value);

	u8 read_high(u8 loc);
	void write_high(u8 loc, u8 value);

private:
	u8 _read(u16 loc);
};