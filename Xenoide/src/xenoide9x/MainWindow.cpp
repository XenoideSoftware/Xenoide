#include "MainWindow.h"

#include <tchar.h>
#include <commdlg.h>
#include <Scintilla.h>

#include "resource.h"

namespace {
    constexpr TCHAR kClassName[] = TEXT("XenoideSimpleMainWindow");

    // ---- Window state -------------------------------------------------------

    HWND hTextEdit = nullptr;
    TCHAR currentFilePath[MAX_PATH] = TEXT("");

    // ---- Find/Replace state -------------------------------------------------

    UINT uFindReplaceMsg = 0;
    HWND hFindDlg = nullptr;
    FINDREPLACE fr = {};
    char szFindText[256] = "";
    char szReplaceText[256] = "";

    // ---- Font helpers -------------------------------------------------------

    void ApplyDialogFont(HWND dialog) {
        const HFONT guiFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        for (HWND child = GetWindow(dialog, GW_CHILD); child != nullptr; child = GetWindow(child, GW_HWNDNEXT)) {
            SendMessage(child, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
        }
    }

    void ConfigureScintillaEditor(HWND hWnd) {
        SendMessage(hWnd, SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>("Courier New"));
        SendMessage(hWnd, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
        SendMessage(hWnd, SCI_STYLECLEARALL, 0, 0);

        SendMessage(hWnd, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
        const int lineNumberMarginWidth = static_cast<int>(
            SendMessage(hWnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<LPARAM>("_99999"))
        );
        SendMessage(hWnd, SCI_SETMARGINWIDTHN, 0, lineNumberMarginWidth);

        SendMessage(hWnd, SCI_SETCARETLINEVISIBLE, 1, 0);
        SendMessage(hWnd, SCI_SETCARETLINEBACK, RGB(245, 245, 245), 0);

        SendMessage(hWnd, SCI_SETTABWIDTH, 4, 0);
        SendMessage(hWnd, SCI_SETINDENT, 4, 0);
        SendMessage(hWnd, SCI_SETUSETABS, 0, 0);
    }

    // ---- Title bar ----------------------------------------------------------

    void UpdateTitleBar(HWND hwnd) {
        TCHAR title[MAX_PATH + 16];
        if (currentFilePath[0] != TEXT('\0')) {
            const TCHAR *fileName = _tcsrchr(currentFilePath, TEXT('\\'));
            fileName = fileName ? fileName + 1 : currentFilePath;
            wsprintf(title, TEXT("%s - Xenoide9X"), fileName);
        } else {
            lstrcpy(title, TEXT("Untitled - Xenoide9X"));
        }
        SetWindowText(hwnd, title);
    }

    // ---- File I/O -----------------------------------------------------------

    // Forward declaration so DoFileSave can call it.
    bool DoFileSaveAs(HWND hwnd);

    bool WriteEditorToFile(HWND hwnd, LPCTSTR path) {
        HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBox(hwnd, TEXT("Failed to save file."), TEXT("Error"), MB_OK | MB_ICONERROR);
            return false;
        }

        const int len = GetWindowTextLengthA(hTextEdit);
        char *buf = new char[len + 1];
        GetWindowTextA(hTextEdit, buf, len + 1);

        DWORD written;
        WriteFile(hFile, buf, static_cast<DWORD>(len), &written, nullptr);
        CloseHandle(hFile);
        delete[] buf;

        SendMessage(hTextEdit, EM_SETMODIFY, FALSE, 0);
        return true;
    }

    bool DoFileSave(HWND hwnd) {
        if (currentFilePath[0] == TEXT('\0'))
            return DoFileSaveAs(hwnd);

        return WriteEditorToFile(hwnd, currentFilePath);
    }

    bool DoFileSaveAs(HWND hwnd) {
        OPENFILENAME ofn = {};
        TCHAR path[MAX_PATH];
        lstrcpy(path, currentFilePath);

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = TEXT("All Files\0*.*\0Text Files\0*.txt\0\0");
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (!GetSaveFileName(&ofn))
            return false;

        lstrcpy(currentFilePath, path);
        const bool ok = WriteEditorToFile(hwnd, currentFilePath);
        if (ok)
            UpdateTitleBar(hwnd);
        return ok;
    }

    // Returns false if the user cancelled (operation should be aborted).
    bool PromptSaveIfModified(HWND hwnd) {
        if (!SendMessage(hTextEdit, EM_GETMODIFY, 0, 0))
            return true;

        const int result = MessageBox(hwnd, TEXT("The current file has unsaved changes. Save before continuing?"), TEXT("Xenoide9X"), MB_YESNOCANCEL | MB_ICONWARNING);

        if (result == IDCANCEL)
            return false;
        if (result == IDNO)
            return true;

        return DoFileSave(hwnd); // IDYES
    }

    void DoFileNew(HWND hwnd) {
        if (!PromptSaveIfModified(hwnd))
            return;

        SetWindowText(hTextEdit, TEXT(""));
        SendMessage(hTextEdit, EM_SETMODIFY, FALSE, 0);
        currentFilePath[0] = TEXT('\0');
        UpdateTitleBar(hwnd);
    }

    void DoFileOpen(HWND hwnd) {
        if (!PromptSaveIfModified(hwnd))
            return;

        OPENFILENAME ofn = {};
        TCHAR path[MAX_PATH] = TEXT("");

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = TEXT("All Files\0*.*\0Text Files\0*.txt\0Source Files\0*.c;*.cpp;*.h\0\0");
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (!GetOpenFileName(&ofn))
            return;

        HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBox(hwnd, TEXT("Failed to open file."), TEXT("Error"), MB_OK | MB_ICONERROR);
            return;
        }

        const DWORD fileSize = GetFileSize(hFile, nullptr);
        char *buf = new char[fileSize + 1];
        DWORD bytesRead;
        ReadFile(hFile, buf, fileSize, &bytesRead, nullptr);
        CloseHandle(hFile);
        buf[bytesRead] = '\0';

        SetWindowTextA(hTextEdit, buf);
        delete[] buf;

        SendMessage(hTextEdit, EM_SETMODIFY, FALSE, 0);
        lstrcpy(currentFilePath, path);
        UpdateTitleBar(hwnd);
    }

    // ---- Find / Replace helpers ---------------------------------------------

    void DoFindText(HWND hwnd) {
        if (szFindText[0] == '\0')
            return;

        const int textLen = GetWindowTextLengthA(hTextEdit);
        char *buf = new char[textLen + 1];
        GetWindowTextA(hTextEdit, buf, textLen + 1);

        DWORD selStart, selEnd;
        SendMessage(hTextEdit, EM_GETSEL, reinterpret_cast<WPARAM>(&selStart), reinterpret_cast<LPARAM>(&selEnd));

        const int findLen = static_cast<int>(strlen(szFindText));
        char *found = nullptr;

        if (fr.Flags & FR_DOWN) {
            found = strstr(buf + selEnd, szFindText);
            if (!found) // wrap around
                found = strstr(buf, szFindText);
        } else {
            // Search backwards from before current selection
            for (int i = static_cast<int>(selStart) - 1; i >= 0; --i) {
                if (strncmp(buf + i, szFindText, static_cast<size_t>(findLen)) == 0) {
                    found = buf + i;
                    break;
                }
            }
        }

        if (found) {
            const int pos = static_cast<int>(found - buf);
            SendMessage(hTextEdit, EM_SETSEL, static_cast<WPARAM>(pos), static_cast<LPARAM>(pos + findLen));
            SendMessage(hTextEdit, EM_SCROLLCARET, 0, 0);
        } else {
            MessageBox(hwnd, TEXT("Text not found."), TEXT("Find / Replace"), MB_OK | MB_ICONINFORMATION);
        }
        delete[] buf;
    }

    void DoReplaceOne(HWND hwnd) {
        // If the current selection already matches, replace it; then find the next one.
        DWORD selStart, selEnd;
        SendMessage(hTextEdit, EM_GETSEL, reinterpret_cast<WPARAM>(&selStart), reinterpret_cast<LPARAM>(&selEnd));

        const int findLen = static_cast<int>(strlen(szFindText));
        if (static_cast<int>(selEnd - selStart) == findLen) {
            const int textLen = GetWindowTextLengthA(hTextEdit);
            char *buf = new char[textLen + 1];
            GetWindowTextA(hTextEdit, buf, textLen + 1);
            char sel[256] = {};
            strncpy(sel, buf + selStart, static_cast<size_t>(findLen));
            delete[] buf;

            if (strcmp(sel, szFindText) == 0)
                SendMessage(hTextEdit, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(szReplaceText));
        }
        DoFindText(hwnd);
    }

    void DoReplaceAll(HWND hwnd) {
        if (szFindText[0] == '\0')
            return;

        // Start from the beginning.
        SendMessage(hTextEdit, EM_SETSEL, 0, 0);

        const int findLen = static_cast<int>(strlen(szFindText));
        const int replaceLen = static_cast<int>(strlen(szReplaceText));
        int count = 0;
        int searchFrom = 0;

        for (;;) {
            const int textLen = GetWindowTextLengthA(hTextEdit);
            char *buf = new char[textLen + 1];
            GetWindowTextA(hTextEdit, buf, textLen + 1);
            const char *found = strstr(buf + searchFrom, szFindText);
            const int pos = found ? static_cast<int>(found - buf) : -1;
            delete[] buf;

            if (pos < 0)
                break;

            SendMessage(hTextEdit, EM_SETSEL, static_cast<WPARAM>(pos), static_cast<LPARAM>(pos + findLen));
            SendMessage(hTextEdit, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(szReplaceText));
            searchFrom = pos + replaceLen;
            ++count;
        }

        if (count == 0) {
            MessageBox(hwnd, TEXT("Text not found."), TEXT("Replace All"), MB_OK | MB_ICONINFORMATION);
        } else {
            TCHAR msg[64];
            wsprintf(msg, TEXT("Replaced %d occurrence(s)."), count);
            MessageBox(hwnd, msg, TEXT("Replace All"), MB_OK | MB_ICONINFORMATION);
        }
    }

    void OpenFindReplaceDialog(HWND hwnd) {
        if (hFindDlg) {
            SetForegroundWindow(hFindDlg);
            return;
        }

        fr = {};
        fr.lStructSize = sizeof(fr);
        fr.hwndOwner = hwnd;
        fr.lpstrFindWhat = szFindText;
        fr.wFindWhatLen = sizeof(szFindText) - 1;
        fr.lpstrReplaceWith = szReplaceText;
        fr.wReplaceWithLen = sizeof(szReplaceText) - 1;
        fr.Flags = FR_DOWN | FR_HIDEWHOLEWORD;

        hFindDlg = ReplaceText(&fr);
    }

    // ---- Window procedure ---------------------------------------------------

    LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        // Handle the modeless Find/Replace dialog messages.
        if (uFindReplaceMsg != 0 && message == uFindReplaceMsg) {
            const LPFINDREPLACE pfr = reinterpret_cast<LPFINDREPLACE>(lParam);
            if (pfr->Flags & FR_DIALOGTERM)
                hFindDlg = nullptr;
            else if (pfr->Flags & FR_FINDNEXT)
                DoFindText(hwnd);
            else if (pfr->Flags & FR_REPLACE)
                DoReplaceOne(hwnd);
            else if (pfr->Flags & FR_REPLACEALL)
                DoReplaceAll(hwnd);
            return 0;
        }

        switch (message) {
        case WM_CREATE:
            uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
            hTextEdit = CreateWindowEx(
                0,
                TEXT("Scintilla"),
                nullptr,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL,
                0,
                0,
                0,
                0,
                hwnd,
                nullptr,
                nullptr,
                nullptr
            );
            ConfigureScintillaEditor(hTextEdit);
            ApplyDialogFont(hwnd);
            UpdateTitleBar(hwnd);
            return 0;

        case WM_SIZE:
            SetWindowPos(hTextEdit, nullptr, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
            return 0;

        case WM_SETFOCUS:
            SetFocus(hTextEdit);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_FILE_NEW:
                DoFileNew(hwnd);
                return 0;

            case ID_FILE_OPEN:
                DoFileOpen(hwnd);
                return 0;

            case ID_FILE_SAVE:
                DoFileSave(hwnd);
                return 0;

            case ID_FILE_SAVE_AS:
                DoFileSaveAs(hwnd);
                return 0;

            case ID_FILE_EXIT:
                SendMessage(hwnd, WM_CLOSE, 0, 0);
                return 0;

            case ID_EDIT_UNDO:
                SendMessage(hTextEdit, WM_UNDO, 0, 0);
                return 0;

            case ID_EDIT_REDO:
                // The built-in EDIT control has no native redo; handled at application level.
                return 0;

            case ID_EDIT_CUT:
                SendMessage(hTextEdit, WM_CUT, 0, 0);
                return 0;

            case ID_EDIT_COPY:
                SendMessage(hTextEdit, WM_COPY, 0, 0);
                return 0;

            case ID_EDIT_PASTE:
                SendMessage(hTextEdit, WM_PASTE, 0, 0);
                return 0;

            case ID_EDIT_FIND_REPLACE:
                OpenFindReplaceDialog(hwnd);
                return 0;

            case ID_EDIT_GOTO_FILE:
                DoFileOpen(hwnd);
                return 0;

            case ID_HELP_ABOUT:
                MessageBox(hwnd, TEXT("Xenoide9X\nNative Win32 IDE"), TEXT("About"), MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            break;

        case WM_CLOSE:
            if (PromptSaveIfModified(hwnd))
                DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }
} // namespace

// ---- Public API ---------------------------------------------------------

bool RegisterMainWindowClass(HINSTANCE instance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APPICON));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName = kClassName;
    return RegisterClass(&wc) != 0;
}

HWND CreateMainWindow(HINSTANCE instance) {
    return CreateWindowEx(0, kClassName, TEXT("Xenoide9X"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, instance, nullptr);
}
