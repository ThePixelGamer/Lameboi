#include "CPU.h"

#include "Gameboy.h"
#include "Memory.h"
#include "util/Log.h"

std::array<std::function<void (CPU*)>, 256> opcodeTable; 

CPU::CPU(Gameboy& core) : bus(core.bus), core(core), interrupt(core.interrupt) {
	// setup opcode table
	opcodeTable.fill(&CPU::invalid);
	
	/// block 0
	// nop
	opcodeTable[0x00] = &CPU::nop;
	
	// stop (todo)
	opcodeTable[0x10] = &CPU::stop;

	// misc
	opcodeTable[0x27] = &CPU::daa;
	opcodeTable[0x2F] = &CPU::cpl;
	opcodeTable[0x37] = &CPU::scf;
	opcodeTable[0x3F] = &CPU::ccf;

	// JR [cc, e8]
	for (u8 i = 0; i < 5; ++i) opcodeTable[0x18 + (i * 0x8)] = &CPU::relJump;

	// LD (nn), SP
	opcodeTable[0x08] = [](CPU* cpu) {
		cpu->loadZW();
		Register tmp;
		tmp = low(cpu->SP);
		cpu->write(cpu->W, cpu->Z, tmp); 
		cpu->inc16(cpu->W, cpu->Z);
		tmp = high(cpu->SP);
		cpu->write(cpu->W, cpu->Z, tmp);
	};
	// LD rr, n16
	opcodeTable[0x01] = [](CPU* cpu) { cpu->loadZW(); cpu->B = cpu->W; cpu->C = cpu->Z; };
	opcodeTable[0x11] = [](CPU* cpu) { cpu->loadZW(); cpu->D = cpu->W; cpu->E = cpu->Z; };
	opcodeTable[0x21] = [](CPU* cpu) { cpu->loadZW(); cpu->H = cpu->W; cpu->L = cpu->Z; };
	opcodeTable[0x31] = &CPU::loadSP;
	
	// LD (rr), A
	opcodeTable[0x02] = [](CPU* cpu) {
		cpu->write(cpu->B, cpu->C, cpu->A);
	};
	opcodeTable[0x12] = [](CPU* cpu) {
		cpu->write(cpu->D, cpu->E, cpu->A);
	};
	opcodeTable[0x22] = [](CPU* cpu) {
		cpu->write(cpu->H, cpu->L, cpu->A); 
		cpu->inc16(cpu->H, cpu->L);
	};
	opcodeTable[0x32] = [](CPU* cpu) {
		cpu->write(cpu->H, cpu->L, cpu->A); 
		cpu->dec16(cpu->H, cpu->L);
	};

	// LD A, (rr)
	opcodeTable[0x0A] = [](CPU* cpu) {
		cpu->A = cpu->read(cpu->B, cpu->C);
	};
	opcodeTable[0x1A] = [](CPU* cpu) {
		cpu->A = cpu->read(cpu->D, cpu->E);
	};
	opcodeTable[0x2A] = [](CPU* cpu) {
		cpu->A = cpu->readHL();
		cpu->inc16(cpu->H, cpu->L);
	};
	opcodeTable[0x3A] = [](CPU* cpu) {
		cpu->A = cpu->readHL();
		cpu->dec16(cpu->H, cpu->L);
	};

	// LD r, n
	for (u8 i = 0; i < 8; ++i) 
		opcodeTable[0x06 + (i * 8)] = [](CPU* cpu) {
			cpu->loadZ();
			cpu->load();
		};

	// ADD HL, rr
	opcodeTable[0x09] = [](CPU* cpu) { cpu->add16(cpu->B, cpu->C); };
	opcodeTable[0x19] = [](CPU* cpu) { cpu->add16(cpu->D, cpu->E); };
	opcodeTable[0x29] = [](CPU* cpu) { cpu->add16(cpu->H, cpu->L); };
	opcodeTable[0x39] = [](CPU* cpu) { cpu->add16(high(cpu->SP), low(cpu->SP)); };

	// INC rr
	opcodeTable[0x03] = [](CPU* cpu) { cpu->inc16(cpu->B, cpu->C); };
	opcodeTable[0x13] = [](CPU* cpu) { cpu->inc16(cpu->D, cpu->E); };
	opcodeTable[0x23] = [](CPU* cpu) { cpu->inc16(cpu->H, cpu->L); };
	opcodeTable[0x33] = [](CPU* cpu) { cpu->SP++; };

	// DEC rr
	opcodeTable[0x0B] = [](CPU* cpu) { cpu->dec16(cpu->B, cpu->C); };
	opcodeTable[0x1B] = [](CPU* cpu) { cpu->dec16(cpu->D, cpu->E); };
	opcodeTable[0x2B] = [](CPU* cpu) { cpu->dec16(cpu->H, cpu->L); };
	opcodeTable[0x3B] = [](CPU* cpu) { cpu->SP--; };

	// INC/DEC r
	for (u8 i = 0; i < 8; ++i) {
		opcodeTable[0x4 + (i * 0x8)] = &CPU::inc;
		opcodeTable[0x5 + (i * 0x8)] = &CPU::dec;
	}

	// rotate A
	opcodeTable[0x07] = [](CPU* cpu) { cpu->rotateLeft(cpu->A, true); cpu->F.Z = false; };
	opcodeTable[0x17] = [](CPU* cpu) { cpu->rotateLeft(cpu->A, false); cpu->F.Z = false; };
	opcodeTable[0x0F] = [](CPU* cpu) { cpu->rotateRight(cpu->A, true); cpu->F.Z = false; };
	opcodeTable[0x1F] = [](CPU* cpu) { cpu->rotateRight(cpu->A, false); cpu->F.Z = false; };

	/// block 1
	// LD r, r
	for (u8 i = 0x40; i < 0x80; ++i) opcodeTable[i] = &CPU::load;

	// halt (todo)
	opcodeTable[0x76] = [](CPU* cpu) {
		//LB_INFO(CPU, "Halt {:04X}", cpu->PC);
		if (cpu->IME) {
			cpu->handler = true;
			cpu->lowPower = true;
		}
		else if (cpu->interrupt.enable.read() & cpu->interrupt.request.read() & 0x1F) {
			cpu->haltBug = true;
		}
		else {
			cpu->lowPower = true;
		}
	};

	/// block 2
	for (u8 i = 0x80; i < 0xC0; ++i) opcodeTable[i] = &CPU::alu;

	/// block 3
	opcodeTable[0xCB] = &CPU::cb;

	// ALU & RST n
	for (u8 i = 0; i < 8; ++i) {
		opcodeTable[0xC6 + (i * 8)] = &CPU::alu;
		opcodeTable[0xC7 + (i * 8)] = &CPU::rst;
	}

	// PUSH rr
	opcodeTable[0xC5] = [](CPU* cpu) { cpu->push(cpu->B, cpu->C); };
	opcodeTable[0xD5] = [](CPU* cpu) { cpu->push(cpu->D, cpu->E); };
	opcodeTable[0xE5] = [](CPU* cpu) { cpu->push(cpu->H, cpu->L); };
	opcodeTable[0xF5] = [](CPU* cpu) { cpu->push(cpu->A, cpu->F); };

	// POP rr
	opcodeTable[0xC1] = [](CPU* cpu) { cpu->pop(cpu->B, cpu->C); };
	opcodeTable[0xD1] = [](CPU* cpu) { cpu->pop(cpu->D, cpu->E); };
	opcodeTable[0xE1] = [](CPU* cpu) { cpu->pop(cpu->H, cpu->L); };
	opcodeTable[0xF1] = [](CPU* cpu) { cpu->pop(cpu->A, cpu->Z); cpu->F = cpu->Z; };

	// RET
	opcodeTable[0xC9] = &CPU::ret;
	opcodeTable[0xD9] = [](CPU* cpu) { cpu->ret(); cpu->IME = true; };
	for (u8 i = 0; i < 4; ++i) opcodeTable[0xC0 + (i * 0x8)] = &CPU::ret;

	// LDH 
	opcodeTable[0xE0] = [](CPU* cpu) { cpu->loadZ(); cpu->write(0xFF00 + cpu->Z, cpu->A); };
	opcodeTable[0xE2] = [](CPU* cpu) { cpu->write(0xFF00 + cpu->C, cpu->A); };
	opcodeTable[0xF0] = [](CPU* cpu) { cpu->loadZ(); cpu->A = cpu->read(0xFF00 + cpu->Z); };
	opcodeTable[0xF2] = [](CPU* cpu) { cpu->A = cpu->read(0xFF00 + cpu->C); };

	// LD
	opcodeTable[0xEA] = [](CPU* cpu) { 
		cpu->loadZW();
		cpu->write(cpu->W, cpu->Z, cpu->A); 
	};
	opcodeTable[0xFA] = [](CPU* cpu) { 
		cpu->loadZW();
		cpu->A = cpu->read(cpu->W, cpu->Z); 
	};
	opcodeTable[0xF9] = [](CPU* cpu) {
		cpu->SP = to16(cpu->H, cpu->L);
	};

	// SP+e8
	auto addSP = [](CPU* cpu) {
		cpu->loadZ();
		u8 neg = 0xFF * bool(cpu->Z & 0x80);
		cpu->_add(cpu->Z, low(cpu->SP));
		cpu->F.Z = false;
		cpu->W = high(cpu->SP) + neg + cpu->F.C;
	};

	opcodeTable[0xE8] = [addSP](CPU* cpu) {
		addSP(cpu);
		cpu->SP = to16(cpu->W, cpu->Z);
	};
	opcodeTable[0xF8] = [addSP](CPU* cpu) {
		addSP(cpu);
		cpu->L = cpu->Z;
		cpu->H = cpu->W;
	};

	// CALL
	opcodeTable[0xC4] = &CPU::call;
	opcodeTable[0xCC] = &CPU::call;
	opcodeTable[0xD4] = &CPU::call;
	opcodeTable[0xDC] = &CPU::call;
	opcodeTable[0xCD] = &CPU::call;

	// JP
	opcodeTable[0xC3] = &CPU::jmp;
	opcodeTable[0xC2] = &CPU::jmp;
	opcodeTable[0xCA] = &CPU::jmp;
	opcodeTable[0xD2] = &CPU::jmp;
	opcodeTable[0xDA] = &CPU::jmp;
	opcodeTable[0xE9] = [](CPU* cpu) { cpu->_jmp(cpu->H, cpu->L); };
	
	// EI/DI
	opcodeTable[0xFB] = [](CPU* cpu) { cpu->IME = true; };
	opcodeTable[0xF3] = [](CPU* cpu) { cpu->IME = false; };

	reset();
	quit = false;
}

void CPU::install(Memory& bus, Model::Type model) {
	auto bios_tag = bus.register_bus(
		nullptr,
		[](void* d, addr, u8 v) { 
			auto cpu = static_cast<CPU*>(d);
			if (v != 0) {
				cpu->inBios = false;
				cpu->bus.Mem()[0xFF50] = v;
			}
		},
		this
	);

	bus.register_io(0x50, bios_tag);

	if (model == Model::CGB) {
		auto key_tag = bus.register_bus(
			[](void* d, addr a) -> u8 {
				auto cpu = static_cast<CPU*>(d);
				switch (a & 0xFF) {
					case 0x4C: return cpu->dmgMode << 2;
					case 0x4D: return (cpu->doubleSpeed << 7) | cpu->speedSwitch; 
				}
				return 0xFF;
			},
			[](void* d, addr a, u8 v) {
				auto cpu = static_cast<CPU*>(d);
				switch (a & 0xFF) {
					case 0x4C:
					if (cpu->inBios)
					cpu->dmgMode = v & 0x4;
					break;
					case 0x4D: cpu->speedSwitch = v & 0x1; break;
				}
			},
			this
		);

		bus.register_io(0x4C, key_tag);
		bus.register_io(0x4D, key_tag);
	}
	
}
	
void CPU::update() {
	// execute
	++instrCount;
	Z = W = 0;
	opcodeTable[IR](this);
	
	// handle interrupts/halt
	auto handleInterrupt = [&]() {
		if (IME && interrupt.pending()) {
			for (u8 i = Interrupt::VBlank; i < Interrupt::_Count; ++i) {
				auto type = Interrupt::Type(i);
				if (interrupt.shouldFire(type)) {
					interrupt.request[type] = false;
					
					stepComponents();
					_call(0x40 + (i * 0x8));
					IME = false;
				}
			}

			return true;
		}

		return false;
	};
		
	if (lowPower) {
		if (handler && handleInterrupt()) {
			handler = false;
			lowPower = false;
		}
		else if (interrupt.pending()) {
			lowPower = false;
		}
		else {
			stepComponents();
			return;
		}
	}
	else {
		handleInterrupt();
	}

	// fetch
	if (haltBug) {
		haltBug = false;
		IR = read(PC).first;
	}
	else {
		IR = read(PC++).first;
	}
}

void CPU::reset() {
	A = B = C = D = E = H = L = 0;
	F = 0;
	SP = PC = 0;
	IR = 0;

	IME = false;
	
	haltBug = false;
	lowPower = false;
	handler = false;
	instrCount = 0;
	inBios = true;
}

std::string CPU::log() {
	return fmt::format("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: {:02X}:{:04X} ({:02X} {:02X} {:02X} {:02X}) Z: {:02X}\n",
			u8(A), u8(F), u8(B), u8(C), u8(D), u8(E), u8(H), u8(L), SP, 0, PC - 1, IR, bus.read(PC), bus.read(PC + 1), bus.read(PC + 2), u8(Z));
}

// IL functions
void CPU::stepComponents() {
	core.step();
}

CPU::ReferenceData CPU::read(addr a) {
	stepComponents();

	u32 src = a;
	// simplify with get bank for addr? add support for wram banks 
	if (a < 0x4000) {
		src |= core.cart.bank0 << sizeof(addr) * 8;
	}
	else if (a < 0x8000) {
		src |= core.cart.bank1 << sizeof(addr) * 8;
	}
	else if ((a & 0xE000) == 0xA) {
		src |= core.cart.bankRam << sizeof(addr) * 8;
	}
	else if (a & 0x8000) {
		src = src_locations[a & 0x7FFF];
	}

	if (inBios) {
		if (a < 0x100) {
			return { bios[a], src};
		}
		else if (a >= 0x200 && a < 0x900 && bios.size() > 0xFF) {
			return { bios[a], src};
		}
	}

	return { bus.read(a), src };
}

void CPU::write(addr a, u8 data) {
	stepComponents();
	bus.write(a, data);
}

void CPU::write(addr a, Register& data) {
	write(a, u8(data));

	// check if we're in 0x8000-0x9fff
	if ((a >> 13) == 0x4) {
		core.ppu.writeIntercept(a, data.second, data);
	}

	if (a & 0x8000) {
		src_locations[a & 0x7FFF] = data.second;
	}
}

void CPU::stop() {
	if (speedSwitch) {
		speedSwitch = false;
		doubleSpeed = !doubleSpeed;
	}
}

void CPU::cb() {
	IR = read(PC++).first;

	Register& r = reg(IR & 0x7);
	bool isZ = &r == &Z;
	
	if (isZ) Z = readHL();

	switch (IR >> 6) {
		case 0:
			switch ((IR >> 3) & 0x7) {
			/* RLC r */ case 0: rotateLeft(r, true); break;
			/* RRC r */ case 1: rotateRight(r, true); break;
			/* RL r  */ case 2: rotateLeft(r); break;
			/* RR r  */ case 3: rotateRight(r); break;
			/* SLA r */ case 4: shiftLA(r); break;
			/* SRA r */ case 5: shiftRA(r); break;
			/* SWAP r*/ case 6: swap(r); break;
			/* SRL r */ case 7: shiftRL(r); break;
			}
		break;
		
		/* BIT b, r */ case 1: bit(r, (IR >> 3) & 0x7); return; // exit early to avoid storing result
		/* RES b, r */ case 2: reset(r, (IR >> 3) & 0x7); break;
		/* SET b, r */ case 3: set(r, (IR >> 3) & 0x7); break;
	}

	if (isZ) writeHL(Z);
}

void CPU::invalid() {
	LB_ERROR(CPU, "Invalid Opcode {:#x}", IR);
	
	// softlock 
	while (!quit) {
		stepComponents();
	}
}

void CPU::alu() {
	Register& r = reg(IR & 0x7);
	if (&r == &Z) {
		// block 2: read [HL], block 3: read n8
		Z = (IR & 0x40) ? readN() : readHL();
	}

	switch ((IR >> 3) & 0x7) {
	/* ADD r, r */ case 0: _add(A, reg(IR & 0x7)); F.Z = A == 0; break;
	/* ADC r, r */ case 1: _add(A, reg(IR & 0x7), F.C); F.Z = A == 0; break;
	/* SUB r, r */ case 2: _sub(A, reg(IR & 0x7)); F.Z = A == 0; break;
	/* SBC r, r */ case 3: _sub(A, reg(IR & 0x7), F.C); F.Z = A == 0; break;
	/* AND r, r */ case 4: _and(); break;
	/* XOR r, r */ case 5: _xor(); break;
	/*  OR r, r */ case 6: _or(); break;
	/*  CP r, r */ case 7: _cp(); break;
	}
}
