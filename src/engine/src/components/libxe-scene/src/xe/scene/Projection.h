
#ifndef __XE_SCENE_PROJECTION_HPP__
#define __XE_SCENE_PROJECTION_HPP__

#include <xe/graphics/Viewport.h>
#include <xe/math/Matrix.h>
#include <xe/math/Vector.h>

namespace xe {
    /**
     * @brief Transforms a four-dimensional vector from World coordinates to Screen coordinates.
     */
    template <typename T> TVector<T, 4> project(const TVector<T, 4> &world, const TMatrix<T, 4, 4> &projViewModel, const Viewport &viewport) {
        const auto window = projViewModel * world;

        auto clip = window;

        // TODO: Add check with machine epsilon
        if (clip.w > T(0.0)) {
            clip /= window.w;
        }

        const auto screen =
            TVector<T, 4>{(clip.x + T(1.0)) * T(0.5) * viewport.size.x + viewport.position.x, (clip.y + T(1.0)) * T(0.5) * viewport.size.y + viewport.position.y, clip.z, clip.w};

        return screen;
    }

    /**
     * @brief Transforms a four-dimensional vector from Screen coordinates to World coordinates.
     */
    template <typename T> TVector<T, 4> unproject(const TVector<T, 4> &screen, const TMatrix<T, 4, 4> &invProjViewModel, const Viewport &viewport) {
        const auto clip = TVector<T, 4>{
            (screen.x - viewport.position.x) / (T(0.5) * viewport.size.x) - T(1.0),
            (screen.y - viewport.position.y) / (T(0.5) * viewport.size.y) - T(1.0),
            screen.z,
            screen.w
        };

        auto world = invProjViewModel * clip;

        // TODO: Add check with machine epsilon
        if (world.w > T(0.0)) {
            world /= world.w;
        }

        return world;
    }
} // namespace xe

#endif
