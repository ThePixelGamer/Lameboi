#pragma once

#include "Util/Types.h"

struct Sprite {
	u8 yPos, xPos, tile;
	struct {
		u8 : 4; //cgb only
		u8 paletteNum : 1;
		u8 xFlip : 1;
		u8 yFlip : 1;
		u8 priority : 1;
	} flags;
};