
#include <windows.h>
#include <iostream>
#include <cstdio>
#include <cstring>

HWND hWndEdit = NULL;

namespace {
    HANDLE hStdinRead, hStdinWrite;
    HANDLE hStdoutRead, hStdoutWrite;

    PROCESS_INFORMATION launchConsoleProcess() {
        SECURITY_ATTRIBUTES processAttribs = {0};
        SECURITY_ATTRIBUTES threadAttribs = {0};
        DWORD creationFlags = DETACHED_PROCESS;

        STARTUPINFOA startupInfo = {0};
        startupInfo.cb = sizeof(STARTUPINFOA);
        startupInfo.dwFlags = STARTF_USESTDHANDLES;
        startupInfo.hStdInput = hStdinRead;
        startupInfo.hStdOutput = hStdoutWrite;
        startupInfo.hStdError = hStdoutWrite;

        PROCESS_INFORMATION processInformation = {0};

        // char commandLine[] = "C:\\TDM-GCC-32-5.1.0-3\\bin\\gcc.exe";
        // "C:\\Windows\\System32\\cmd.exe"
        // "C:\\Command.com"
        if (CreateProcess(
                NULL,
                "C:\\Command.com",
                &processAttribs,
                &threadAttribs,
                TRUE,
                creationFlags,
                NULL /*lpEnvironment*/,
                NULL /*lpCurrentDirectory*/,
                &startupInfo,
                &processInformation
            ) == 0L) {
            char buffer[256] = {0};
            const int errorCode = GetLastError();
            sprintf(buffer, "Create process failed!\nError code: %d\n", errorCode);

            MessageBoxA(NULL, buffer, "Error", MB_OK | MB_ICONERROR);

            return processInformation;
        }

        return processInformation;
    }

    DWORD WINAPI displayConsoleOutputThreadProc(LPVOID lpParameter) {
        DWORD dwRead = 0;
        char buffer[16] = {0};

        while (ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead > 0) {
            buffer[dwRead] = '\0';
            printf("%s", buffer);

            ::SendMessage(hWndEdit, EM_SETSEL, -1, -1);
            ::SendMessage(hWndEdit, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(buffer));
        }

        /*
        const char *buffer = "Hello from the console output thread!\n";
        ::SendMessage(hWndEdit, EM_SETSEL, -1, -1);
        ::SendMessage(hWndEdit, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(buffer));
        */

        return 0;
    }

    WNDPROC editWndProc = NULL;

    bool startTracking = false;
    int promptStart = 0;

    DWORD selStart = 0, selEnd = 0;

    LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

        switch (message) {
        case WM_CHAR:
            /*
            if (wParam == -1) {
                LRESULT result = CallWindowProc(editWndProc, hwnd, message, wParam, lParam);

                // Extract current written line from the Edit control
                // char* line = "dir";

                // get the string contained in the control
                int len = GetWindowTextLength(hwnd);
                std::string text;
                text.resize(len);
                GetWindowText(hwnd, (LPSTR)text.c_str(), text.size());

                // get the current caret position
                ::SendMessage(hwnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

                // extract the prompt
                std::string line = text.substr(promptStart, selStart - promptStart - 1);
                // line = "hola";

                MessageBox(NULL, text.c_str(), "Xenoide", MB_OK);
                MessageBox(NULL, ("\"" + line + "\"").c_str(), "Xenoide", MB_OK);

                DWORD dw = 0;
                WriteFile(hStdinWrite, line.c_str(), line.size(), &dw, NULL);
                WriteFile(hStdinWrite, "\r\n", 2, &dw, NULL);

                startTracking = false;

                return result;
            }
            else {
                UINT uScanCode = (lParam >> 16) & 0xFF;
                WORD dwChar = 0;
                int result = ToAscii(wParam, uScanCode, NULL, &dwChar, 0);

                if (result == 1) {
                    char buffer[2] = {
                        (char)  LOBYTE(dwChar),
                        '\0'
                    };

                    DWORD dw = 0;
                    WriteFile(hStdinWrite, buffer, sizeof(buffer), &dw, NULL);
                }
                else if (result == 2) {
                    char buffer[3] = {
                        (char)  LOBYTE(dwChar),
                        (char)  HIBYTE(dwChar),
                        '\0'
                    };

                    DWORD dw = 0;
                    WriteFile(hStdinWrite, buffer, sizeof(buffer), &dw, NULL);
                }
            }
            */

            if (wParam == VK_RETURN) {
                ::SendMessage(hwnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
                ::SendMessage(hwnd, EM_SETSEL, promptStart, selStart);
                ::SendMessage(hwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(""));
                startTracking = false;

                DWORD dw = 0;
                WriteFile(hStdinWrite, "\r\n", 2, &dw, NULL);

                return 0L;
            } else {
                char buffer[2] = {(char)wParam, '\0'};
                DWORD dw = 0;
                WriteFile(hStdinWrite, buffer, 1, &dw, NULL);

                return CallWindowProc(editWndProc, hwnd, message, wParam, lParam);
            }
            break;

        case WM_KEYDOWN:
            if (!startTracking) {
                ::SendMessage(hwnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
                promptStart = (int)selStart;

                startTracking = true;
            }

            break;
        }

        return CallWindowProc(editWndProc, hwnd, message, wParam, lParam);
    }

    void setupConsole() {
        // create pipes to grab stdin and stdout
        SECURITY_ATTRIBUTES pipeAttributes = {0};
        pipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        pipeAttributes.bInheritHandle = TRUE;

        CreatePipe(&hStdinRead, &hStdinWrite, &pipeAttributes, 0);
        CreatePipe(&hStdoutRead, &hStdoutWrite, &pipeAttributes, 0);

        // setup process
        PROCESS_INFORMATION processInformation = launchConsoleProcess();
        if (processInformation.hProcess == NULL) {
            return;
        }

        HANDLE hThread = CreateThread(NULL, 0, &displayConsoleOutputThreadProc, NULL, CREATE_SUSPENDED, NULL);
        ResumeThread(hThread);

        // WaitForSingleObject(processInformation.hProcess, INFINITE);
        // DWORD exitCode = 0;
        // GetExitCodeProcess(processInformation.hProcess, &exitCode);
    }

    const TCHAR kWindowClassName[] = TEXT("Xenoide10XMainWindow");

    LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_CREATE:
            hWndEdit = CreateWindowEx(
                0,
                TEXT("EDIT"),
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
                0,
                0,
                0,
                0,
                hwnd,
                NULL,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL
            );

            editWndProc = (WNDPROC)SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)ConsoleWndProc);

            return 0;

        case WM_SIZE:
            if (hWndEdit) {
                MoveWindow(hWndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    bool RegisterMainWindowClass(HINSTANCE instance) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = MainWindowProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = kWindowClassName;
        return RegisterClass(&wc) != 0;
    }

    HWND CreateMainWindow(HINSTANCE instance) {
        return CreateWindowEx(0, kWindowClassName, TEXT("Xenoide10X"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, instance, NULL);
    }
} // namespace

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    if (!RegisterMainWindowClass(hInstance)) {
        MessageBox(NULL, TEXT("Failed to register window class."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    HWND mainWindow = CreateMainWindow(hInstance);
    if (mainWindow == NULL) {
        MessageBox(NULL, TEXT("Failed to create main window."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);

    setupConsole();

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
