
#pragma once

#include <glaze/gl.h>
#include <GLFW/glfw3.h>
#include <string>

struct InputState {
    bool keyLeftPress = false;
    bool keyRightPress = false;
    bool keyUpPress = false;
    bool keyDownPress = false;
    bool keyEscPress = false;
};

typedef GLAZE_PROC (*PlatformGetProcAddress)(const char *procname);

class Platform {
public:
    Platform();

    virtual ~Platform();

    bool initialize(const std::string &title, int width, int height);

    InputState pollInputState();

    void setTitle(const std::string &title);

    void swapBuffers();

    PlatformGetProcAddress getGLProcAddressProcedure();

    int getWindowWidth() const {
        return windowWidth;
    }

    int getWindowHeight() const {
        return windowHeight;
    }

    float getAspectRatio() const {
        return static_cast<float>(getWindowWidth()) / static_cast<float>(getWindowHeight());
    }

private:
    GLFWwindow *window = nullptr;
    int windowWidth = 0;
    int windowHeight = 0;
};
