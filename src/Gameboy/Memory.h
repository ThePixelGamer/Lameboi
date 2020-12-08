#pragma once

//https://gbdev.gg8.se/wiki/articles/Memory_Map

#include <array>
#include <iostream>
#include <mutex>

#include "Util/Types.h"
#include "ppu/Palette.h"
#include "ppu/Sprite.h"

struct Gameboy;

class Memory {
	Gameboy& gb;

	//regs
	std::array<u8, 0x1000> WRAMBank0; //0xC000
	std::array<u8, 0x1000> WRAMBank1; //0xD000
	std::array<u8, 0x1E00> mirrorWRAM; //0xE000
	u8 DMA; //0xFF46 DMA Transfer and Start Address

public:
	bool boot; //0xFF50, need to move this

private:
	std::array<u8, 0x60> undocumented; //0xFEA0
	std::array<u8, 0x7F> HRAM; //0xFF80

	//internal
	bool memoryRead = false; //bit of a hack to use the read function during dma
	bool inDMA = false;
	u16 currentDMA = 0; //what address the dma is currently on
	u8 DMAOffset = 0; //which byte we're currently copying
	u8 DMAByte = 0; //what byte is on the bus for reads below hram

public:
	Memory(Gameboy&);
	void clean();
	void update(); //oam dma

	void Write(u16 loc, u8 value);
	u8 Read(u16 loc);
};