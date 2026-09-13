
#ifndef __XE_INPUT_INPUTMANAGER_HPP__
#define __XE_INPUT_INPUTMANAGER_HPP__

namespace xe {
    class KeyboardStatus;
    class MouseStatus;
    class ControllerStatus;

    /**
     * @brief Provides low-level device status scanning capabilities
     *
     * @todo Add API for connected device detection
     */
    class InputManager {
    public:
        virtual ~InputManager();

        /**
         * @brief Scans the status of all recognized input devices.
         */
        virtual void poll() = 0;

        virtual KeyboardStatus getKeyboardStatus() = 0;

        virtual MouseStatus getMouseStatus() = 0;

        virtual int getControllerCount() = 0;

        virtual ControllerStatus getControllerStatus(const int index) = 0;
    };
} // namespace xe

#endif
