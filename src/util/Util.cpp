#include "Log.h"

namespace Log {

std::string FormatEntryMessage(const Entry& entry) {
	std::string_view filename = entry.filename;
	auto srcPos = std::min(filename.find("src/"), filename.find("src\\"));
	return fmt::format("[{}] [{}] {}:{}:{}: {}\n", entry.level_name, entry.class_name,
		filename.substr(srcPos), entry.line, entry.function, entry.message);
}
	
}
