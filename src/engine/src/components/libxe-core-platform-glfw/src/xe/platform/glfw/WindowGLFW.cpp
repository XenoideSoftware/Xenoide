
#include "WindowGLFW.h"

#include "GraphicsContextGLFW.h"
#include "InputManagerGLFW.h"
#include <GLFW/glfw3.h>
#include <memory>

#include <iostream>
#include <map>

namespace xe {
    void static errorCallback(int error, const char *description) {
        std::cout << "GLFW errorCallback: " << error << ": " << description << '\n';
    }

    static std::map<int, int> mapToHints(const GraphicsContext::Descriptor &descriptor) {
        std::map<int, int> hints;

        auto version = get_version(descriptor.backend);

        if (descriptor.backend == GraphicsBackend::GL_41) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        } else {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
            glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        }

        hints[GLFW_CONTEXT_VERSION_MAJOR] = std::get<0>(version);
        hints[GLFW_CONTEXT_VERSION_MINOR] = std::get<1>(version);

        hints[GLFW_OPENGL_FORWARD_COMPAT] = GL_TRUE;
        hints[GLFW_OPENGL_PROFILE] = GLFW_OPENGL_CORE_PROFILE;

        hints[GLFW_DEPTH_BITS] = get_depth_bit_count(descriptor.depthBufferFormat);
        hints[GLFW_DOUBLEBUFFER] = GL_TRUE;

        auto colorBits = get_color_bit_count(descriptor.frameBufferFormat);

        hints[GLFW_RED_BITS] = std::get<0>(colorBits);
        hints[GLFW_GREEN_BITS] = std::get<1>(colorBits);
        hints[GLFW_BLUE_BITS] = std::get<2>(colorBits);
        hints[GLFW_ALPHA_BITS] = std::get<3>(colorBits);
        hints[GLFW_RESIZABLE] = GL_FALSE;

        return hints;
    }

    class WindowGLFWImpl : public WindowGLFW {
    public:
        WindowGLFWImpl(const GraphicsContext::Descriptor &contextDescriptor, const std::string &title, const Vector2i &windowSize, const bool fullScreen) {
            if (usageCount++ == 0) {
                glfwInit();
            }

            std::cout << "[GLFW] Initializing GLFW ..." << '\n';

            glfwSetErrorCallback(errorCallback);

            auto hints = mapToHints(contextDescriptor);

            for (auto &pair : hints) {
                glfwWindowHint(pair.first, pair.second);
            }

            std::cout << "[GLFW] Creating Window/Context ..." << '\n';
            glfwWindow = glfwCreateWindow(windowSize.X, windowSize.Y, title.c_str(), fullScreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

            if (!glfwWindow) {
                std::string desc;
                desc.resize(256);

                const int error = glfwGetError((const char **)desc.c_str());

                desc = "Error code " + std::to_string(error) + ": " + desc;

                throw std::runtime_error(desc);
            }

            std::cout << "[GLFW] Making Context current ..." << '\n';
            glfwMakeContextCurrent(glfwWindow);

            graphicsContext = std::make_unique<GraphicsContextGLFW>(glfwWindow, contextDescriptor);
            inputManager = std::make_unique<InputManagerGLFW>(glfwWindow);
        }

        ~WindowGLFWImpl() override {
            if (glfwWindow) {
                glfwMakeContextCurrent(nullptr);
                glfwDestroyWindow(glfwWindow);
            }

            if (--usageCount == 0) {
                glfwTerminate();
            }
        }

        Vector2i getSizeInPixels() const override {
            int width = 0, height = 0;

            glfwGetWindowSize(glfwWindow, &width, &height);

            return {width, height};
        }

        void setTitle(const std::string &title) override {
            ::glfwSetWindowTitle(glfwWindow, title.c_str());
        }

        GraphicsContext *getContext() const override {
            return graphicsContext.get();
        }

        InputManager *getInputManager() const override {
            return inputManager.get();
        }

    private:
        GLFWwindow *glfwWindow = nullptr;

    private:
        std::unique_ptr<GraphicsContextGLFW> graphicsContext;
        std::unique_ptr<InputManagerGLFW> inputManager;
        static int usageCount;
    };

    int WindowGLFWImpl::usageCount = 0;

    WindowGLFW::~WindowGLFW() {
    }

    std::unique_ptr<WindowGLFW>
    WindowGLFW::create(const GraphicsContext::Descriptor &contextDescriptor, const std::string &title, const Vector2i &windowSize, const bool fullScreen) {
        return std::make_unique<WindowGLFWImpl>(contextDescriptor, title, windowSize, fullScreen);
    }
} // namespace xe
