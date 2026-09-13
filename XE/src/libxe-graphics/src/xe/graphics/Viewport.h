
#ifndef __XE_GRAPHICS_VIEWPORT_HPP__
#define __XE_GRAPHICS_VIEWPORT_HPP__

#include <xe/math/Vector.h>

namespace xe {
    struct Viewport {
        Vector2i position;
        Vector2i size;
    };
} // namespace xe

#endif
