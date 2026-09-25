
#include <xe/app/Platform.h>

#include <glaze/gl.h>

#include <cassert>
#include <cstring>
#include <iostream>

Platform::Platform() {
    glfwInit();
}

Platform::~Platform() {
    if (window) {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
}

bool Platform::initialize(const std::string &title, int width, int height) {
    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        std::cout << "Can't open a Window" << '\n';

        const char description[1024] = {};
        const char *desc = &description[0];

        glfwGetError(&desc);

        std::cout << description << '\n';

        return false;
    }

    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwMakeContextCurrent(window);

    return true;
}

InputState Platform::pollInputState() {
    glfwPollEvents();

    InputState inputState;

    inputState.keyEscPress = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    inputState.keyLeftPress = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    inputState.keyRightPress = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    inputState.keyUpPress = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    inputState.keyDownPress = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    return inputState;
}

void Platform::setTitle(const std::string &title) {
    glfwSetWindowTitle(window, title.c_str());
}

void Platform::swapBuffers() {
    glfwSwapBuffers(window);
}

PlatformGetProcAddress Platform::getGLProcAddressProcedure() {
    return (PlatformGetProcAddress)glfwGetProcAddress;
}
