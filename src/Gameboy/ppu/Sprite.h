#pragma once

#include "Util/Types.h"

struct Sprite {
	u8 xPos, yPos, tile;
	bool useOBP1;
	bool xFlip, yFlip;
	bool behindBG;
	u8 lowerX3Byte;

	Sprite() :
		xPos(0),
		yPos(0),
		tile(0), 
		useOBP1(false),
		xFlip(false),
		yFlip(false),
		behindBG(false),
		lowerX3Byte(0)
	{}

	u8 read(u8 reg) {
		switch (reg) {
			case 0: return yPos;
			case 1: return xPos;
			case 2: return tile;
			case 3: return (behindBG << 7) | (yFlip << 6) | (xFlip << 5) | (useOBP1 << 4) | lowerX3Byte;

			default:
				//log
				return 0xFF;
		}
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
			case 0: yPos = value; break;
			case 1: xPos = value; break;
			case 2: tile = value; break;
			case 3: 
				behindBG = (value & 0x80);
				yFlip = (value & 0x40);
				xFlip = (value & 0x20);
				useOBP1 = (value & 0x10);
				lowerX3Byte = (value & 0xF);
				break;

			default:
				//log
				break;
		}
	}
};