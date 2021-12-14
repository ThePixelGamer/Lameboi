#pragma once

#include <array>
#include "util/Types.h"

class Interrupt;

// todo: move to file?
namespace GB {

enum Button : u8 {
	Up,
	Down,
	Left,
	Right,
	Start,
	Select,
	B,
	A,
	NumButtons
};

};

class Joypad {
	Interrupt& interrupt;

	//button states
	std::array<bool, GB::NumButtons> buttonStates;

	//registers
	bool selectDirect; 
	bool selectButton;
	bool b1, b2, b3, b4;

public:
	Joypad(Interrupt& interrupt);

	void clean();

	void pressButton(GB::Button button);
	void releaseButton(GB::Button button);
	bool getButtonState(GB::Button button);

	u8 read();
	void write(u8);

private:
	bool& _mapToButton(GB::Button button);

	template<GB::Button Button>
	void updateButton(bool down);
};