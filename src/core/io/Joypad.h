#pragma once

#include <array>
#include "util/Types.h"

class Interrupt;

class Joypad {
public:
	//button states
	enum Button : u8 {
		Down, Up, Left, Right,
		Start, Select, B, A,
		NumButtons
	};

	constexpr static const char* names[] = {
		"Down", "Up", "Left", "Right",
		"Start", "Select", "B", "A"
	};

private:
	Interrupt& interrupt;
	std::array<bool, NumButtons> buttonStates;
	bool selectDirect, selectButton;

public:
	Joypad(Interrupt& interrupt);

	void clean();

	u8 read();
	void write(u8);

private:
	void updateButton(Button button, bool down);
};