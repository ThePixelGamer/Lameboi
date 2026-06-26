#include "MBC.h"

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

			case Header::MBC2_ROM: return MBC2;
			case Header::MBC2_BATTERY: return MBC2 | BATTERY;

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

	// todo: memorymap the rom
	std::ifstream romFile(romPath, std::ifstream::binary);

	if (!romFile) {
		LB_ERROR(MBC, "Failed to open {}", romPath.string());
		return false;
	}

	romSize = std::filesystem::file_size(romPath);
	rom = new u8[romSize];
	romFile.read((char*)rom, romSize);
	
	std::size_t maxRomSize = getHeader()->getMaxRomBanks() * ROM_BANK_SIZE;
	if (romSize != maxRomSize) {
		LB_WARN(MBC, "Rom filesize mismatch with cartridge header size: {}", maxRomSize);
	}

	_initHW();

	if (has(RAM)) {
		ramSize = (is(MBC2)) ? 512 : getHeader()->getMaxRamBanks() * RAM_BANK_SIZE;

		// hack: hijack end of ram to save MBC3's RTC Registers 
		if (has(TIMER)) {
			ramSize += MBC3::RTC_REGS;
		}

		ram = new u8[ramSize];
	}

	// todo: memorymap the ram
	if (has(BATTERY)) {
		auto savePath = getSavePath();
		if (std::filesystem::exists(savePath)) {
			std::size_t savSize = std::filesystem::file_size(savePath);

			std::ifstream ramFile(savePath, std::ifstream::binary);
			ramFile.read((char*)ram, ramSize);

			if (has(TIMER)) {
				// todo: update rtc registers since last game run
				auto delta = std::filesystem::file_time_type::clock::now() - std::filesystem::last_write_time(savePath);

			}
		}
	}

	return true;
}

void Cartridge::unload() {
	if (has(BATTERY)) {
		std::ofstream ramFile(getSavePath(), std::ofstream::binary);
		ramFile.write((char*)ram, ramSize);
	}

	delete ram;
	ramSize = 0;
	delete rom;
	romSize = 0;
}

void Cartridge::writeBank0(u16 offset, u8 data) { if (mbc) mbc->writeBank0(offset, data); }
void Cartridge::writeBank1(u16 offset, u8 data) { if (mbc) mbc->writeBank1(offset, data); }
void Cartridge::writeRam(u16 offset, u8 data) { if (mbc) mbc->writeRam(offset, data); }

u8 Cartridge::readBank0(u16 offset) { return (mbc) ? mbc->readBank0(offset) : 0x00; }
u8 Cartridge::readBank1(u16 offset) { return (mbc) ? mbc->readBank1(offset) : 0x00; }
u8 Cartridge::readRam(u16 offset) { return (mbc) ? mbc->readRam(offset) : 0x00; }