#pragma once

#include <mutex>
#include <fmt/format.h>
#include "util/Types.h"

namespace Log {

struct Entry {
    const char* class_name = nullptr;
    const char* level_name = nullptr;
    const char* filename = nullptr;
    u32 line = 0;
    const char* function = nullptr;
    std::string message;
};

std::string FormatEntryMessage(const Entry& entry);

template <typename... Args>
void Write(const char* log_class, const char* log_level, const char* filename, u32 line, 
                    const char* function, const char* format, const Args&... args) {
    static std::mutex logMutex;
    std::lock_guard guard(logMutex);

    Entry entry{
        log_class,
        log_level,
        filename,
        line,
        function,
        fmt::format(fmt::runtime(format), args...) // message
    };

    auto str = FormatEntryMessage(entry);

    // add printing to a file and add color support for console
    fmt::print("{}", str);
}

} // namespace Log

#define LB_LOG(log_class, log_level, ...) \
    Log::Write(#log_class, #log_level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define LB_INFO(log_class, ...) LB_LOG(log_class, Info, __VA_ARGS__)
#define LB_WARN(log_class, ...) LB_LOG(log_class, Warning, __VA_ARGS__)
#define LB_ERROR(log_class, ...) LB_LOG(log_class, Error, __VA_ARGS__)
