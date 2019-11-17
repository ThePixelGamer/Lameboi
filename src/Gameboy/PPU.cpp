#include "PPU.h"

#include "Gameboy.h"

#include <algorithm> //std::fill
#include <iterator> //std::size
#include <iostream>

PPU::PPU(Gameboy& t_gb) : gb(t_gb) {
	Clean();
}

void PPU::Clean() {
	std::fill(tileData, std::end(tileData), 0);
	//std::fill(bg1Map, std::end(bg1Map), 0);
	//std::fill(bg2Map, std::end(bg2Map), 0);
}

void PPU::LineRender() {

}

void PPU::DecodePalette(Palette& paletteData) {
	auto convert = [&](u8 color)->u32 {
		switch(color & 0b11) {
			case 0b00: return 0xFFFFFFFF;
			case 0b01: return 0xAAAAAAFF;
			case 0b10: return 0x555555FF;
			case 0b11: return (paletteData != gb.mem.BGP) ? 0x00000000 : 0x000000FF;
			default: std::cerr << "Unknown pixel value" << std::endl; return 0;
		}
	};

	Color t_color = {convert(paletteData.color0), convert(paletteData.color1), convert(paletteData.color2), convert(paletteData.color3)};
	if(paletteData == gb.mem.BGP) {
		BGP = t_color;
	}
	if(paletteData == gb.mem.OBP0) {
		OBP0 = t_color;
	}
	if(paletteData == gb.mem.OBP1) {
		OBP1 = t_color;
	}
}

void PPU::WriteTileData(u16 byteLoc, u8 bottom) { //
	u32 t_pixels[8];

	u8 top = u8(gb.mem.Read(byteLoc + ((byteLoc % 2) ? -1 : 1)));
	if(byteLoc % 2) {
		std::swap(top, bottom);
	}

	for(u8 i = 0; i < 8; i++) {
		switch((((top >> (7 - i)) << 1) & 0b10) | ((bottom >> (7 - i)) & 0b1)) {
			case 0b00: t_pixels[i] = 0xFFFFFFFF; break;
			case 0b01: t_pixels[i] = 0xAAAAAAFF; break;
			case 0b10: t_pixels[i] = 0x555555FF; break;
			case 0b11: t_pixels[i] = 0x000000FF; break;
			default: std::cerr << "Unknown pixel value" << std::endl; break;
		}
	}

	u8 tileByte = (byteLoc & 0xff) % 16;  
	u8 tileX = (byteLoc >> 4) & 0xf;
	u8 tileY = byteLoc >> 8;
	u16 pixelLoc = (tileY * 1024) + (tileX * 8) + ((tileByte / 2) * 128);
	std::copy(t_pixels, std::end(t_pixels), tileData + pixelLoc);
}

void PPU::WriteBGTile(u16 byteLoc, u8 value) {
	u32* dest;

	if(byteLoc >= 0x9C00) {
		dest = bgMap2;
		byteLoc -= 0x9C00;
	}
	else {
		dest = bgMap1;
		byteLoc -= 0x9800;
	}

	dest += ((byteLoc / 32) * 2048) + ((byteLoc % 32) * 8);

	u32* tileCoord = tileData + ((value / 16) * 1024) + ((value % 16) * 8) + ((gb.mem.LCDC.tileSet) ? 0x0 : 0x800);
	for(u8 i = 0; i < 8; i++) {
		for(u8 j = 0; j < 8; j++) {
			switch((tileCoord[j] & 0xff00) >> 8) {
				case 0xFF: dest[j] = BGP.color0; break;
				case 0xAA: dest[j] = BGP.color1; break;
				case 0x55: dest[j] = BGP.color2; break;
				case 0x00: dest[j] = BGP.color3; break;
				default: std::cerr << "Unknown pixel value" << std::endl; break;
			}
		}
		//next line
		tileCoord += 128; 
		dest += 256;
	}
}