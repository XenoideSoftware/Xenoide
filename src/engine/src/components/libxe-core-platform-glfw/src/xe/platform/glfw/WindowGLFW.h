
#ifndef __XE_GRAPHICS_WINDOWGLFW_HPP__
#define __XE_GRAPHICS_WINDOWGLFW_HPP__

#include <memory>
#include <string>
#include <xe/graphics/GraphicsContext.h>
#include <xe/graphics/GraphicsDevice.h>
#include <xe/graphics/IWindow.h>
#include <xe/input/InputManager.h>

namespace xe {
    /**
     * Abstraction layer around a GLFW Window.
     */
    class WindowGLFW : public IWindow {
    public:
        virtual ~WindowGLFW();

        virtual Vector2i getSizeInPixels() const = 0;

        virtual GraphicsContext *getContext() const = 0;

        virtual InputManager *getInputManager() const = 0;

    public:
        static std::unique_ptr<WindowGLFW> create(const xe::GraphicsContext::Descriptor &descriptor, const std::string &title, const Vector2i &windowSize, const bool fullScreen);
    };
} // namespace xe

#endif
