
#include "Renderer.h"
#include <Windows.h>
#include <stdexcept>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

BOOL InitWindowClass(const HINSTANCE hInstance, const LPSTR className) {
    WNDCLASS wc = {};

    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.lpfnWndProc = WindowProc;
    wc.hbrBackground = (HBRUSH)COLOR_3DFACE;

    if (::RegisterClass(&wc)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
int main() {
    using namespace TestApp;

    const LPSTR className = const_cast<char *>("D3D11_01_Triangle");

    HINSTANCE hInstance = ::GetModuleHandle(NULL);

    try {
        // instanciar la clase de ventana
        if (::InitWindowClass(hInstance, className) == FALSE) {
            throw std::runtime_error("No se pudo crear una clase de ventana");
        }

        // crear la ventana
        HWND hWnd = ::CreateWindow(className, "Direct3D11 01: Triangle", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

        ::ShowWindow(hWnd, SW_SHOW);

        Renderer renderer;

        renderer.Initialize(hWnd);

        while (TRUE) {
            MSG msg;

            if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    break;
                } else {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            } else {
                renderer.Render();
                // TODO: Do rendering
            }
        }

        ::CloseWindow(hWnd);
        ::DestroyWindow(hWnd);

        return 0;
    } catch (const std::exception &exp) {
        ::MessageBoxA(NULL, exp.what(), "", MB_OK | MB_ICONERROR);
        return 1;
    }
}
