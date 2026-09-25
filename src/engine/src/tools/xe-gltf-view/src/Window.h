#pragma once

#include <SDL2/SDL.h>
#include <imgui.h>
#include <cgltf.h>

class Window {
public:
    Window();

    ~Window();

    bool pollInput();

    void update();

    void swapBuffers();

    bool initialize();

    // called once per frame, just after the buffers are cleared
    void prepareUI();

    // called once per frame
    void drawUI();

    void setData(cgltf_data *newData);

private:
    int initializeSDL();

    int initializeOpenGL();

    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
    ImGuiContext *imGuiContext = nullptr;
    cgltf_data *data = nullptr;
};
