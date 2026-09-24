#include <windows.h>

#include "MainWindow.h"
#include "resource.h"

#include <sstream>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    HINSTANCE hScintillaModule = ::LoadLibrary(TEXT("Scintilla.dll"));

    if (hScintillaModule == nullptr) {
        DWORD const error = GetLastError();
        std::ostringstream oss;
        oss << "Failed to load Scintilla.dll. Make sure it is in the same directory as the executable.\nError code: " << error;
        MessageBox(nullptr, oss.str().c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);

        return -1;
    }

    if (!RegisterMainWindowClass(hInstance)) {
        MessageBox(nullptr, TEXT("Failed to register window classes."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    HWND mainWindow = CreateMainWindow(hInstance);
    if (mainWindow == nullptr) {
        MessageBox(nullptr, TEXT("Failed to create the main window."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);

    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        if (!TranslateAccelerator(mainWindow, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ::DestroyAcceleratorTable(hAccel);
    ::FreeLibrary(hScintillaModule);
    return static_cast<int>(msg.wParam);
}
