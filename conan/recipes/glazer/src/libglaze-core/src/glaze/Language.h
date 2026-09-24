#pragma once

namespace glaze {

/**
 * @brief Target output language for the facade.
 * Extended description: selects which generator the BindingGenerator runs for
 * a given request. A single generate() call may request both languages; the
 * facade runs them sequentially and merges the returned files.
 */
enum class Language {
    C,
    Cpp,
};

} // namespace glaze
