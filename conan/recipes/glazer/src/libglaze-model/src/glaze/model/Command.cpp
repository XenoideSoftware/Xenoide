#include "glaze/model/Command.h"

namespace glaze::model {

std::optional<std::string> getClass(const Command &command) {
    if (command.params.empty()) {
        return std::nullopt;
    }
    const auto &first = command.params.front();
    if (!hasClass(first)) {
        return std::nullopt;
    }
    return first.classStr;
}

} // namespace glaze::model
