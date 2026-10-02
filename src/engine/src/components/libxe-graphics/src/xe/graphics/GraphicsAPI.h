
#pragma once

#include <tuple>

namespace xe {
    enum class GraphicsBackend { Auto, GL_41, GL_ES_2 };

    inline std::tuple<int, int> get_version(const GraphicsBackend backend) {
        switch (backend) {
        case GraphicsBackend::GL_41:
            return {4, 1};
        case GraphicsBackend::GL_ES_2:
            return {2, 0};
        default:
            return {0, 0};
        }
    }
} // namespace xe
