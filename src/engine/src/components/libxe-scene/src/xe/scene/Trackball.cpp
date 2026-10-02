
#include <xe/scene/Trackball.h>

namespace xe {
    Trackball::Trackball(const Vector2i &size) {
        this->resize(size);
    }

    void Trackball::beginDrag(const Vector2i &position) {
        dragBegin = position;
    }

    void Trackball::drag(const Vector2i &position) {
        dragCurrent = position;
    }

    void Trackball::endDrag(const Vector2i &position) {
        dragEnd = position;
    }

    void Trackball::resize(const Vector2i &size) {
        vsphere.setScreenSize(size);
    }

    Rotation<float> Trackball::computeRotation() const {
        return {0.0f, {0.0f, 0.0f, 0.0f}};
    }

    Vector2 Trackball::scalePosition(const Vector2i &position) const {
        // map position to the [-1, 1]^2 range domain
        const Vector2 position_f = xe::cast<float>(position);
        const Vector2 size_f = xe::cast<float>(vsphere.getScreenSize());
        const Vector2 mappedPosition = (position_f / (0.5f * size_f)) - Vector2{1.0f};

        return mappedPosition;
    }

    Vector3 Trackball::computeSpherePosition(const Vector2 &scaledPosition) const {
        const auto sp = scaledPosition;
        const auto sp_length_squared = norm2(sp);

        if (sp_length_squared > 1.0f) {
            return normalize(Vector3{sp.x, sp.y, 0.0f});
        }

        return {sp.x, sp.y, std::sqrt(1.0f - sp_length_squared)};
    }
} // namespace xe
