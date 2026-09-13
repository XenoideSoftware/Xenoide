
#include <xe/input/DeviceStatus.h>

namespace xe {
    BinaryState KeyboardStatus::getState(KeyCode code) const {
        return m_keys[static_cast<int>(code)];
    }

    void KeyboardStatus::setState(const KeyCode code, const BinaryState state) {
        m_keys[static_cast<int>(code)] = state;
    }

    Vector2i MouseStatus::getPosition() const {
        return m_position;
    }

    void MouseStatus::setPosition(const Vector2i &position) {
        m_position = position;
    }

    BinaryState MouseStatus::getState(const MouseButton button) const {
        return m_buttons[static_cast<int>(button)];
    }

    void MouseStatus::setState(const MouseButton button, const BinaryState state) {
        m_buttons[static_cast<int>(button)] = state;
    }

    BinaryState ControllerStatus::getState(const ControllerButton button) const {
        return m_buttons[static_cast<int>(button)];
    }

    void ControllerStatus::setState(const ControllerButton button, const BinaryState state) {
        m_buttons[static_cast<int>(button)] = state;
    }

    Vector2 ControllerStatus::getState(const ControllerJoystick joystick) const {
        return m_joysticks[static_cast<int>(joystick)];
    }

    void ControllerStatus::setState(const ControllerJoystick joystick, const Vector2 &state) {
        m_joysticks[static_cast<int>(joystick)] = state;
    }
} // namespace xe
