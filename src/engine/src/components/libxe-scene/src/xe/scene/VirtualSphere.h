
#ifndef __XE_SCENE_VIRTUALSPHERE_HPP__
#define __XE_SCENE_VIRTUALSPHERE_HPP__

#include <xe/math/Rotation.h>
#include <xe/math/Vector.h>

namespace xe {
    /**
     * @brief Virtual sphere tied into a screen. Used as an utility in the Trackball class.
     */
    class VirtualSphere {
    public:
        VirtualSphere();

        VirtualSphere(const Vector2i &screenSize);

        Vector2i getScreenSize() const {
            return screenSize;
        }

        void setScreenSize(const Vector2i &value);

        Vector3 computePointAt(const Vector2i &screenPosition) const;

        bool operator==(const VirtualSphere &rhs) const;

        bool operator!=(const VirtualSphere &rhs) const {
            return !(*this == rhs);
        }

    private:
        Vector2i screenSize;
    };
} // namespace xe

#endif
