
#ifndef __XE_GRAPHICS_GL_INPUTMANAGERGLFW_HPP__
#define __XE_GRAPHICS_GL_INPUTMANAGERGLFW_HPP__

#include <GLFW/glfw3.h>
#include <xe/input/DeviceStatus.h>
#include <xe/input/InputManager.h>

namespace xe {
    class InputManagerGLFW : public InputManager {
    public:
        explicit InputManagerGLFW(GLFWwindow *window);

        virtual void poll() override;

        virtual KeyboardStatus getKeyboardStatus() override {
            return m_keyboardStatus;
        }

        virtual MouseStatus getMouseStatus() override;

        virtual int getControllerCount() override;

        virtual ControllerStatus getControllerStatus(const int index) override;

    private:
        void fillKeyboardStatus();

    private:
        GLFWwindow *m_windowGLFW = nullptr;
        KeyboardStatus m_keyboardStatus = {};
    };
} // namespace xe

#endif
