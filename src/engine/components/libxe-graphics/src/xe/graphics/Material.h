
#ifndef __XE_GRAPHICS_MATERIAL_HPP__
#define __XE_GRAPHICS_MATERIAL_HPP__

#include <array>
#include <cstddef>
#include <xe/Predef.h>

namespace xe {
    class Texture;

    struct MaterialLayer {
        const Texture *texture = nullptr;
    };

    /**
     * @brirf Fill mode for polygon
     */

    /**
     * @brief Graphics Device current render states
     */
    struct MaterialRenderState {};

    /**
     * @brief Describes the generic visual appearance of all the objects. For other properties, use uniforms.
     * @note Very unstable interface.
     */
    class Material {
    public:
        ~Material();

        std::array<MaterialLayer, 8> layers;
        int layerCount = 0;
        MaterialRenderState renderState;
    };
} // namespace xe

#endif
