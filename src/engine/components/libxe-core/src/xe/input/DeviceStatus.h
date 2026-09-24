
#ifndef __XE_INPUT_INPUTSTATE_HPP__
#define __XE_INPUT_INPUTSTATE_HPP__

#include <xe/math/Vector.h>

namespace xe {
    enum class BinaryState { Press, Release };

    enum class KeyCode {
        KeyEsc,
        KeyF1,
        KeyF2,
        KeyF3,
        KeyF4,
        KeyF5,
        KeyF6,
        KeyF7,
        KeyF8,
        KeyF9,
        KeyF10,
        KeyF11,
        KeyF12,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        Key0,
        KeyBackspace,
        KeyTab,
        KeyQ,
        KeyW,
        KeyE,
        KeyR,
        KeyT,
        KeyY,
        KeyU,
        KeyI,
        KeyO,
        KeyP,
        KeyA,
        KeyS,
        KeyD,
        KeyF,
        KeyG,
        KeyH,
        KeyJ,
        KeyK,
        KeyL,
        KeyZ,
        KeyX,
        KeyC,
        KeyV,
        KeyB,
        KeyN,
        KeyM,
        KeyEnter,
        KeyLCtrl,
        KeyAlt,
        KeySpace,
        KeyAltGr,
        KeyRCtrl,
        KeyLShift,
        KeyRShift,
        KeyUp,
        KeyDown,
        KeyLeft,
        KeyRight,
        KeyHome,
        KeyEnd,
        KeySupr,
        KeyInsert,
        Max
    };

    class KeyboardStatus {
    public:
        BinaryState getState(KeyCode code) const;

        void setState(const KeyCode code, const BinaryState state);

        bool isPressed(const KeyCode code) const {
            return getState(code) == BinaryState::Press;
        }

        bool isReleased(const KeyCode code) const {
            return getState(code) == BinaryState::Release;
        }

    private:
        BinaryState m_keys[static_cast<int>(KeyCode::Max)] = {};
    };

    enum class MouseButton { Left, Right, Middle, Max };

    class MouseStatus {
    public:
        Vector2i getPosition() const;

        void setPosition(const Vector2i &position);

        BinaryState getState(const MouseButton button) const;

        void setState(const MouseButton button, const BinaryState state);

    private:
        Vector2i m_position;
        BinaryState m_buttons[static_cast<int>(MouseButton::Max)];
    };

    enum class ControllerButton { LPad_Left, LPad_Right, LPad_Up, LPad_Down, RPad_Left, RPad_Right, RPad_Up, RPad_Down, L1, R1, L2, R2, Start, Select, Max };

    enum class ControllerJoystick { Left, Right, Max };

    class ControllerStatus {
    public:
        BinaryState getState(const ControllerButton button) const;

        void setState(const ControllerButton button, const BinaryState state);

        Vector2 getState(const ControllerJoystick joystick) const;

        void setState(const ControllerJoystick joystick, const Vector2 &state);

    private:
        BinaryState m_buttons[static_cast<int>(ControllerButton::Max)];
        Vector2 m_joysticks[static_cast<int>(ControllerJoystick::Max)];
    };
} // namespace xe

#endif
