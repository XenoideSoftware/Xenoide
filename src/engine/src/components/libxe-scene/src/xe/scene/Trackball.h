
#ifndef __XE_SCENE_TRACKBALL_HPP__
#define __XE_SCENE_TRACKBALL_HPP__

#include <xe/math/Quaternion.h>
#include <xe/math/Rotation.h>
#include <xe/math/Vector.h>
#include <xe/scene/VirtualSphere.h>

namespace xe {
    /**
     * @brief Generate rotation information
     */
    class Trackball {
    public:
        Trackball(const Vector2i &size);

        void beginDrag(const Vector2i &position);
        void drag(const Vector2i &position);
        void endDrag(const Vector2i &position);

        void resize(const Vector2i &size);

        Rotation<float> computeRotation() const;

        Vector2i getDragBegin() const {
            return dragBegin;
        }

        Vector2i getDragEnd() const {
            return dragEnd;
        }

        Vector2i getDragCurrent() const {
            return dragCurrent;
        }

        VirtualSphere getVirtualSphere() const {
            return vsphere;
        }

    private:
        Vector2 scalePosition(const Vector2i &position) const;

        Vector3 computeSpherePosition(const Vector2 &normalizedPosition) const;

    private:
        VirtualSphere vsphere;
        Vector2i dragBegin;
        Vector2i dragEnd;
        Vector2i dragCurrent;
    };
} // namespace xe

#endif
