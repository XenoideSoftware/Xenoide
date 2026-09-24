
#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#define XE_LOG_INFO(msg, ...) fmt::print(msg, ##__VA_ARGS__)
#define XE_LOG_WARNING(msg, ...) fmt::print(fg(fmt::color::yellow), msg, ##__VA_ARGS__)
#define XE_LOG_ERROR(msg, ...) fmt::print(fg(fmt::color::crimson), msg, ##__VA_ARGS__)

namespace xe {
    void logInfo(const std::string &msg);
    void logWarning(const std::string &msg);
    void logError(const std::string &msg);
} // namespace xe
