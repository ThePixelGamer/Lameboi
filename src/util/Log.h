#pragma once

#include <mutex>
#include <fmt/format.h>
#include "util/Types.h"

namespace Log {

enum class Level : u8 {
    Info,     
    Warning,  
    Error,    

    Count
};

enum class Class : u8 {
    CPU,
    PPU,
    Audio,
    Memory,
    IO,
    MBC,
    Joypad,
    Serial,
    Timer,
    Frontend,
    CG,
    Input,
    Util,

    Count
};

// todo: move both string funcs to a source file
inline const char* LevelToString(Level log_level) {
#define LVL(x) \
    case Level::x: \
        return #x

    switch (log_level) {
        LVL(Info);
        LVL(Warning);
        LVL(Error);
        case Level::Count:
            break;
    }

#undef LVL
    return "Invalid";
}

inline const char* ClassToString(Class log_class) {
#define CLS(x) \
    case Class::x: \
        return #x

    switch (log_class) {
        CLS(CPU);
        CLS(PPU);
        CLS(Audio);
        CLS(Memory);
        CLS(IO);
        CLS(MBC);
        CLS(Joypad);
        CLS(Serial);
        CLS(Timer);
        CLS(Frontend);
        CLS(CG);
        CLS(Input);
        CLS(Util);
        case Class::Count:
            break;
    }

#undef CLS
    return "Invalid";
}

struct Entry {
    const char* class_name = nullptr;
    const char* level_name = nullptr;
    const char* filename = nullptr;
    u32 line = 0;
    const char* function = nullptr;
    std::string message;
};

// todo: move this to a source file
inline std::string FormatMessage(const Entry& entry) {
    return fmt::format("{} [{}] {}:{}:{}: {}\n", entry.class_name, entry.level_name, 
                       entry.filename, entry.line, entry.function, entry.message);
}

// switch to a queue?
template <typename... Args>
void Write(Class log_class, Level log_level, const char* filename, u32 line, 
                    const char* function, const char* format, const Args&... args) {
    static std::mutex logMutex;
    std::lock_guard guard(logMutex);

    // switch to C++20?
    Entry entry{
        ClassToString(log_class), // class_name
        LevelToString(log_level), // level_name
        filename,
        line,
        function,
        fmt::format(format, args...) // message
    };

    std::string str = FormatMessage(entry);

    // add printing to a file and add color support for console
    fmt::print(str);
}

} // namespace Log

#define LB_LOG(log_class, log_level, ...) \
    Log::Write(Log::Class::log_class, Log::Level::log_level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define LB_INFO(log_class, ...) LB_LOG(log_class, Info, __VA_ARGS__)
#define LB_WARN(log_class, ...) LB_LOG(log_class, Warning, __VA_ARGS__)
#define LB_ERROR(log_class, ...) LB_LOG(log_class, Error, __VA_ARGS__)