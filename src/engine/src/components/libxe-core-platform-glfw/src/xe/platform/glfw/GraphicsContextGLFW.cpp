
#include "GraphicsContextGLFW.h"

#include <cassert>

namespace xe {
    GraphicsContextGLFW::GraphicsContextGLFW(GLFWwindow *glfwWindow, Descriptor descriptor) {
        assert(glfwWindow);

        this->glfwWindow = glfwWindow;
        this->descriptor = descriptor;
    }

    GraphicsContextGLFW::~GraphicsContextGLFW() {
    }

    GraphicsContext::GetProcAddressGL GraphicsContextGLFW::getProcAddressFunctionGL() const {
        return (GraphicsContext::GetProcAddressGL)glfwGetProcAddress;
    }

    GraphicsContext::Descriptor GraphicsContextGLFW::getDescriptor() const {
        return descriptor;
    }

    void GraphicsContextGLFW::present() {
        glfwSwapBuffers(glfwWindow);
    }
} // namespace xe
