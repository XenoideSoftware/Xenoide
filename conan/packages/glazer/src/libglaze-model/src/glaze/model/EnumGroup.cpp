#include "glaze/model/EnumGroup.h"

namespace glaze::model {

const Enum *findEnum(const EnumGroup &group, std::string_view name) noexcept {
    for (const auto &entry : group.enums) {
        if (entry.first == name) {
            return &entry.second;
        }
    }
    return nullptr;
}

} // namespace glaze::model
