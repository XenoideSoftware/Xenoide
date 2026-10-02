
#include "glcore3-context.h"

namespace xe {
    /**
     * @brief Resolve a ProgramGL from a shader pool slot, or nullptr on validation failure.
     *
     * Performs the full index-range + slot-alive + generation-match check. Returns a borrowed
     * pointer into the pool; the pointer is valid until the next mutation of the shader-program pool.
     */
    const ProgramGL *tryProgramExtract(const std::vector<OptSlot<ProgramGL>> &pool, ShaderHandle handle) {
        uint32_t const index = handle.index();
        if (index >= pool.size()) {
            return nullptr;
        }
        OptSlot<ProgramGL> const &slot = pool[index];
        if (!slot.obj) {
            return nullptr;
        }
        if (slot.gen != handle.gen()) {
            return nullptr;
        }
        return &*slot.obj;
    }

} // namespace xe
