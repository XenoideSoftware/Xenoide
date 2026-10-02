#include "glaze/model/CommandParam.h"

namespace glaze::model {

std::optional<std::string> baseType(const CommandParam &param) noexcept {
    for (const auto &part : param.typeParts) {
        if (part.empty()) {
            continue;
        }
        if (part == "const" || part == "*" || part == "**" || part == "void") {
            continue;
        }
        return part;
    }
    return std::nullopt;
}

int pointerIndirection(const CommandParam &param) noexcept {
    int count = 0;
    for (const auto &part : param.typeParts) {
        for (char ch : part) {
            if (ch == '*') {
                ++count;
            }
        }
    }
    return count;
}

bool isConst(const CommandParam &param) noexcept {
    for (const auto &part : param.typeParts) {
        if (part == "const") {
            return true;
        }
    }
    return false;
}

bool isPointer(const CommandParam &param) noexcept {
    return pointerIndirection(param) > 0;
}

bool isVoid(const CommandParam &param) noexcept {
    for (const auto &part : param.typeParts) {
        if (part == "void") {
            return true;
        }
    }
    return false;
}

bool hasGroup(const CommandParam &param) noexcept {
    return param.group.has_value() && !param.group->empty();
}

bool hasClass(const CommandParam &param) noexcept {
    return param.classStr.has_value() && !param.classStr->empty();
}

} // namespace glaze::model
