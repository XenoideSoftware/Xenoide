#include "MainWindow.h"
#include <windows.h>

namespace {
    constexpr WORD ID_FILE_NEW = 101;
    constexpr WORD ID_FILE_OPEN = 102;
    constexpr WORD ID_FILE_SAVE = 103;
    constexpr WORD ID_FILE_SAVEAS = 104;
    constexpr WORD ID_FILE_EXIT = 109;

    constexpr WORD ID_EDIT_UNDO = 201;
    constexpr WORD ID_EDIT_REDO = 202;
    constexpr WORD ID_EDIT_CUT = 203;
    constexpr WORD ID_EDIT_COPY = 204;
    constexpr WORD ID_EDIT_PASTE = 205;

    constexpr WORD ID_HELP_ABOUT = 901;
} // namespace

MainWindow::MainWindow() {
    setup.wndClassEx.lpszClassName = L"XenoideWinLambMain";
    setup.title = L"Xenoide";
    setup.size = {900, 600};

    on_message(WM_CREATE, [this](wl::params) -> LRESULT {
        buildMenu();
        statusbar.create(this).add_resizable_part(1).set_text(L"Ready", 0);
        return 0;
    });

    on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
        statusbar.adjust(p);
        return 0;
    });

    on_message(WM_COMMAND, [this](wl::params p) -> LRESULT {
        onCommand(LOWORD(p.wParam));
        return 0;
    });
}

void MainWindow::buildMenu() {
    mainMenu = wl::menu{CreateMenu()};

    {
        wl::menu m = mainMenu.append_submenu(L"&File");
        m.append_item(ID_FILE_NEW, L"&New\tCtrl+N")
            .append_item(ID_FILE_OPEN, L"&Open...\tCtrl+O")
            .append_item(ID_FILE_SAVE, L"&Save\tCtrl+S")
            .append_item(ID_FILE_SAVEAS, L"Save &As...")
            .append_separator()
            .append_item(ID_FILE_EXIT, L"E&xit");
    }

    {
        wl::menu m = mainMenu.append_submenu(L"&Edit");
        m.append_item(ID_EDIT_UNDO, L"&Undo\tCtrl+Z")
            .append_item(ID_EDIT_REDO, L"&Redo\tCtrl+Y")
            .append_separator()
            .append_item(ID_EDIT_CUT, L"Cu&t\tCtrl+X")
            .append_item(ID_EDIT_COPY, L"&Copy\tCtrl+C")
            .append_item(ID_EDIT_PASTE, L"&Paste\tCtrl+V");
    }

    {
        wl::menu m = mainMenu.append_submenu(L"&Help");
        m.append_item(ID_HELP_ABOUT, L"&About...");
    }

    SetMenu(hwnd(), mainMenu.hmenu());
}

void MainWindow::onCommand(WORD id) {
    switch (id) {
    case ID_FILE_NEW:
        statusbar.set_text(L"New", 0);
        break;
    case ID_FILE_OPEN:
        statusbar.set_text(L"Open", 0);
        break;
    case ID_FILE_SAVE:
        statusbar.set_text(L"Save", 0);
        break;
    case ID_FILE_SAVEAS:
        statusbar.set_text(L"Save As", 0);
        break;
    case ID_FILE_EXIT:
        PostQuitMessage(0);
        break;
    case ID_EDIT_UNDO:
        statusbar.set_text(L"Undo", 0);
        break;
    case ID_EDIT_REDO:
        statusbar.set_text(L"Redo", 0);
        break;
    case ID_EDIT_CUT:
        statusbar.set_text(L"Cut", 0);
        break;
    case ID_EDIT_COPY:
        statusbar.set_text(L"Copy", 0);
        break;
    case ID_EDIT_PASTE:
        statusbar.set_text(L"Paste", 0);
        break;
    case ID_HELP_ABOUT:
        MessageBoxW(hwnd(), L"Xenoide WinLamb Shell", L"About", MB_OK | MB_ICONINFORMATION);
        break;
    }
}
