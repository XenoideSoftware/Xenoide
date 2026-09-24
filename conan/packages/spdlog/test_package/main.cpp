#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    auto logger = spdlog::stdout_color_mt("test");
    logger->set_level(spdlog::level::info);
    logger->info("hello from spdlog {}", SPDLOG_VERSION);
    return 0;
}
