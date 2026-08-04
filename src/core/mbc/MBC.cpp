#include "MBC.h"

#include "core/Memory.h"

#include "MBC1.h"
#include "MBC3.h"
#include "MBC5.h"

#include "util/Log.h"

std::string Cartridge::getHWName(Hardware hw) {
	std::string name;
	switch (hw & HW_MASK) {
		case BARE: name = "BARE"; break;
		case MBC1: name = "MBC1"; break;
		case MBC2: name = "MBC2"; break;
		case MMM01: name = "MMM01"; break;
		case MBC3: name = "MBC3"; break;
		case MBC5: name = "MBC5"; break;
		case MBC6: name = "MBC6"; break;
		case MBC7: name = "MBC7"; break;
		case Camera: name = "Camera"; break;
		case BANDAI_TAMA5: name = "BANDAI_TAMA5"; break;
		case HuC3: name = "HuC3"; break;
		case HuC1: name = "HuC1"; break;
		default: name = ""; break;
	}

	if (hw & TIMER) {
		name += "+TIMER";
	}
	if (hw & RUMBLE) {
		name += "+RUMBLE";
	}
	if (hw & RAM) {
		name += "+RAM";
	}
	if (hw & BATTERY) {
		name += "+BATTERY";
	}

	return name;
}

void Cartridge::_initHW() {
	components = [type = getHeader()->cartType]() {
		switch (type) {
			case Header::ROM_ONLY: return BARE;
			case Header::ROM_RAM: return BARE | RAM;
			case Header::ROM_RAM_BATTERY: return BARE | RAM | BATTERY;

			case Header::MBC1_ROM: return Hardware::MBC1;
			case Header::MBC1_RAM: return Hardware::MBC1 | RAM;
			case Header::MBC1_RAM_BATTERY: return Hardware::MBC1 | RAM | BATTERY;

			case Header::MBC2_ROM: return MBC2 | RAM;
			case Header::MBC2_BATTERY: return MBC2 | RAM | BATTERY;

			case Header::MMM01_ROM: return MMM01;
			case Header::MMM01_RAM: return MMM01 | RAM;
			case Header::MMM01_RAM_BATTERY: return MMM01 | RAM | BATTERY;

			case Header::MBC3_TIMER_BATTERY: return Hardware::MBC3 | TIMER | BATTERY;
			case Header::MBC3_TIMER_RAM_BATTERY: return Hardware::MBC3 | TIMER | RAM | BATTERY;
			case Header::MBC3_ROM: return Hardware::MBC3;
			case Header::MBC3_RAM: return Hardware::MBC3 | RAM;
			case Header::MBC3_RAM_BATTERY: return Hardware::MBC3 | RAM | BATTERY;

			case Header::MBC5_ROM: return Hardware::MBC5;
			case Header::MBC5_RAM: return Hardware::MBC5 | RAM;
			case Header::MBC5_RAM_BATTERY: return Hardware::MBC5 | RAM | BATTERY;
			case Header::MBC5_RUMBLE: return Hardware::MBC5 | RUMBLE;
			case Header::MBC5_RUMBLE_RAM: return Hardware::MBC5 | RUMBLE | RAM;
			case Header::MBC5_RUMBLE_RAM_BATTERY: return Hardware::MBC5 | RUMBLE | RAM | BATTERY;

			case Header::MBC6: return MBC6;
			case Header::MBC7_SENSOR_RUMBLE_RAM_BATTERY: return MBC7;
			case Header::POCKET_CAMERA: return Camera;
			case Header::BANDAI_TAMA5: return BANDAI_TAMA5;
			case Header::HUC3: return HuC3;
			case Header::HUC1_RAM_BATTERY: return HuC1;
				
			default: 
				LB_ERROR(MBC, "Unknown Cartridge Type Code: %x\n", (std::underlying_type_t<Header::Type>)type); 
				return BARE;
		}
	}();

	mbc = [&]() -> std::unique_ptr<MBC> {
		switch (components & HW_MASK){
			case BARE: return std::make_unique<Bare>(*this);
			case MBC1: return std::make_unique<::MBC1>(*this);
			case MBC3: return std::make_unique<::MBC3>(*this);
			case MBC5: return std::make_unique<::MBC5>(*this);

				// todo: implement other cart types
			case MBC2:
			case MMM01:
			case MBC6:
			case MBC7:
			case Camera:
			case BANDAI_TAMA5:
			case HuC3:
			case HuC1:
			default:
				LB_ERROR(MBC, "Unimplemented Cartridge Type: %s\n", getHWName(components));
				return nullptr;
		}
	}();
}

bool Cartridge::load(const std::filesystem::path& romPath) {
	if (!std::filesystem::exists(romPath)) {
		LB_ERROR(MBC, "File {} does not exist", romPath.string());
		return false;
	}

	romName = romPath.stem().string();

	romSize = std::filesystem::file_size(romPath);
	rom_backing = MemoryMap{romPath, Access::RW, 0};
	rom = rom_backing.map(); 
    bus.addressSpace.split(0, Memory::PAGE_SIZE * 4);
    bus.addressSpace.split(0x4000, Memory::PAGE_SIZE * 4);
	
	switchBank0(0);
	switchBank1(1);

	std::size_t maxRomSize = getHeader()->getMaxRomBanks() * ROM_BANK_SIZE;
	if (romSize != maxRomSize) {
		LB_WARN(MBC, "Rom filesize mismatch with cartridge header size: {}", maxRomSize);
	}

	_initHW();

	if (has(RAM)) {
		ramSize = (is(MBC2)) ? 512 : getHeader()->getMaxRamBanks() * RAM_BANK_SIZE;

		// hack: hijack end of ram to save MBC3's RTC Registers 
		if (has(TIMER)) {
			//ramSize += MBC3::RTC_REGS;
		}
	}

	// todo: memorymap the ram
	if (has(BATTERY)) {
		auto savePath = getSavePath();
		if (!std::filesystem::exists(savePath)) {
			{ std::ofstream{savePath}; }
			std::filesystem::resize_file(savePath, ramSize);
		}

		if (std::filesystem::exists(savePath)) {
			ram_backing = MemoryMap{savePath, Access::RW, ramSize};
			ram = ram_backing.map();

			if (has(TIMER)) {
				// todo: update rtc registers since last game run
				auto delta = std::filesystem::file_time_type::clock::now() - std::filesystem::last_write_time(savePath);

			}
		}
	}
	else if (ramSize != 0) {
		ram_backing = MemoryMap{ramSize};
		ram = ram_backing.map();
	}
	
	connected = true;
	return true;
}

void Cartridge::switchBank0(u8 bank) {
	rom0 = {};
	rom0 = rom_backing.map(bus.Mem(), Memory::PAGE_SIZE * 4 * bank, Memory::PAGE_SIZE * 4);
}

void Cartridge::switchBank1(u8 bank) {
	bank &= u8(getHeader()->getMaxRomBanks() - 1);
	rom1 = {};
	rom1 = rom_backing.map(bus.Mem() + Memory::PAGE_SIZE * 4, Memory::PAGE_SIZE * 4 * bank, Memory::PAGE_SIZE * 4);
}

void Cartridge::enableRam(bool enable) {
	size_t offset = (enable) ? Memory::PASS_THROUGH : Memory::NOP;
	ram0_tag = {};
	ram1_tag = {};
	ram0_tag = bus.tag_backing.map(bus.Tags() + Memory::PAGE_SIZE * 0xA, offset, Memory::PAGE_SIZE);
	ram1_tag = bus.tag_backing.map(bus.Tags() + Memory::PAGE_SIZE * 0xB, offset, Memory::PAGE_SIZE);
}

void Cartridge::switchRam(u8 bank) {
	bank &= u8(getHeader()->getMaxRamBanks() - 1);
	ram0 = {};
	ram0 = ram_backing.map(bus.Mem() + Memory::PAGE_SIZE * 0xA, Memory::PAGE_SIZE * 2 * bank, Memory::PAGE_SIZE * 2);
}

void Cartridge::unload() {
	// only unload if load() has been called, rom should be non-null after that call
	if (connected) {
		/*
		if (has(BATTERY)) {
			std::ofstream ramFile(getSavePath(), std::ofstream::binary);
			ramFile.write((char*)ram, ramSize);
		}
		*/

		//delete ram;
		ramSize = 0;
		ram_backing.close();
		ram = {};
		ram0 = {};
		ram0_tag = {};
		ram1_tag = {};

		//delete rom;
		romSize = 0;

		rom_backing.close();
		rom = {};
		rom0 = {};
		rom1 = {};
		mbc = {};

		connected = false;
	}
}

Bare::Bare(Cartridge& hw) : MBC(hw), tag_backing(Memory::PAGE_SIZE * 8) {
	auto& bus = hw.bus;

	{
		auto tag_data = tag_backing.map();
		std::ranges::fill(tag_data.get<Memory::BusTag>(), tag_data.get<Memory::BusTag>() + Memory::PAGE_SIZE * 8, Memory::BusTag{.read = false, .write = true});
	}

	bus.addressSpace.split(Memory::ADDRESS_SPACE, Memory::PAGE_SIZE * 8);
	rom_tag = tag_backing.map(bus.Tags(), 0, Memory::PAGE_SIZE * 8);
	
	bus.addressSpace.split(Memory::ADDRESS_SPACE + 0xA000, Memory::PAGE_SIZE);
	bus.addressSpace.split(Memory::ADDRESS_SPACE + 0xB000, Memory::PAGE_SIZE);
	hw.enableRam(hw.has(Cartridge::RAM));
}
