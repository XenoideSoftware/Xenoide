
#include "main_window.h"
#include <tchar.h>
#include <winlamb/font.h>

#include <cderr.h>
#include <commdlg.h>
#include <winlamb/sysdlg.h>
#include <map>
#include <cassert>
#include <filesystem>
#include <set>
#include <numeric>
#include "resource.h"

#include "xenoide/core/FileService.h"
#include "xenoide/core/StringUtil.h"

#include "SciLexer.h"
#include "ILexer.h"

#include "find_dialog.h"

namespace {
    // Child window IDs for the dock layout and its contents.
    enum {
        IDC_DOCKS = 5100,
        IDC_LEFT_DOCK,
        IDC_DOCUMENTS,
        IDC_BOTTOM_DOCK,
        IDC_FILES_PAGE,
        IDC_OUTLINE_PAGE,
        IDC_EDITOR1_PAGE,
        IDC_OUTPUT_PAGE,
        IDC_LOGS_PAGE,
        IDC_TB,
        IDC_TB_MENU,
        IDC_REBAR
    };

    constexpr const TCHAR *statusReady = _T("Ready");

    enum {
        ID_FILE_NEW = 101,
        ID_FILE_OPEN,
        ID_FILE_SAVE,
        ID_FILE_SAVEAS,
        ID_FILE_EXIT,

        ID_EDIT_UNDO,
        ID_EDIT_REDO,
        ID_EDIT_CUT,
        ID_EDIT_COPY,
        ID_EDIT_PASTE,
        ID_EDIT_FIND,
        ID_EDIT_REPLACE,

        ID_HELP_ABOUT,

        // for the toolbar menu
        ID_FILE,
        ID_EDIT,
        ID_HELP
    };

    wl::tstring to_tstring(const std::filesystem::path &path) {
#if defined(_UNICODE) || defined(UNICODE)
        return path.wstring();
#else
        return path.string();
#endif
    }

    wl::tstring makeAcceleratorCaption(BYTE fVirt, WORD key) {
        wl::tstring str;

        if (fVirt & FCONTROL) {
            if (!str.empty()) {
                str += _T("+");
            }

            str += _T("Ctrl");
        }

        if (fVirt & FSHIFT) {
            if (!str.empty()) {
                str += _T("+");
            }

            str += _T("Shift");
        }

        if (fVirt & FALT) {
            if (!str.empty()) {
                str += _T("+");
            }

            str += _T("Alt");
        }

        if (!str.empty()) {
            str += _T("+");
        }

        str += static_cast<TCHAR>(key);

        return str;
    }
} // namespace

MainWindow::MainWindow() {
    HINSTANCE hInst = GetModuleHandle(nullptr);

    setup.wndClassEx.lpszClassName = _T("Xenoide");
    setup.wndClassEx.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
    setup.wndClassEx.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
    setup.title = _T("Xenoide");
    setup.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    setup.size = {1100, 700};

    on_message(WM_CREATE, [this](wl::params) -> LRESULT {
        // buildMenuToolbar();
        buildMenu();

        buildStatusBar();
        buildAcceleratorTable();
        // buildRebar();
        // buildToolbars();
        buildUI();

        return 0;
    });

    on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
        SendMessage(hwndRebar, WM_SIZE, 0, 0);
        statusbar.adjust(p);
        layout();

        return 0;
    });

    on_message(WM_MENUSELECT, [this](wl::params p) -> LRESULT {
        const WORD itemId = LOWORD(p.wParam);
        const WORD flags = HIWORD(p.wParam);

        // Menu closed: wParam == 0xFFFF and lParam == 0.
        if (flags == 0xFFFF && p.lParam == 0) {
            statusbar.set_text(statusReady, 0);
            return 0;
        }

        // Submenu, separator, system-menu entry, or no current item:
        // nothing useful to describe, so blank the hint area.
        if (flags & (MF_POPUP | MF_SEPARATOR | MF_SYSMENU)) {
            statusbar.set_text(_T(""), 0);
            return 0;
        }

        statusbar.set_text(lookupMenuHint(itemId), 0);
        return 0;
    });

    on_message(WM_CLOSE, [this](wl::params) -> LRESULT {
        controller.onClose();
        return 0;
    });

    on_notify(IDC_TB_MENU, TBN_DROPDOWN, [this](wl::params p) -> LRESULT {
        auto const &nmtb = *reinterpret_cast<NMTOOLBAR *>(p.lParam);
        auto const hdr = &nmtb.hdr;

        RECT rc{};
        SendMessageW(hdr->hwndFrom, TB_GETRECT, static_cast<WPARAM>(nmtb.iItem), reinterpret_cast<LPARAM>(&rc));
        MapWindowPoints(hdr->hwndFrom, HWND_DESKTOP, reinterpret_cast<POINT *>(&rc), 2);

        wl::menu m;
        switch (nmtb.iItem) {
        case ID_FILE:
            m = filePopupMenu;
            break;
        case ID_EDIT:
            m = editPopupMenu;
            break;
        case ID_HELP:
            m = helpPopupMenu;
            break;
        default:
            return TBDDRET_NODEFAULT;
        }

        TrackPopupMenu(m.hmenu(), TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom, 0, this->hwnd(), nullptr);
        return TBDDRET_DEFAULT;
    });

    on_command(ID_FILE_NEW, [this](wl::params) -> LRESULT {
        controller.onNewFileCommand();
        return 0;
    });

    on_command(ID_FILE_OPEN, [this](wl::params) -> LRESULT {
        controller.onOpenFileCommand();
        return 0;
    });

    on_command(ID_FILE_SAVE, [this](wl::params) -> LRESULT {
        controller.onSaveFileCommand();
        return 0;
    });

    on_command(ID_FILE_SAVEAS, [this](wl::params) -> LRESULT {
        controller.onSaveAsFileCommand();
        return 0;
    });

    on_command(ID_FILE_EXIT, [this](wl::params) -> LRESULT {
        PostMessage(this->hwnd(), WM_CLOSE, 0, 0);
        return 0;
    });

    on_command(ID_EDIT_UNDO, [this](wl::params) -> LRESULT {
        controller.onEditorUndoCommand();
        return 0;
    });

    on_command(ID_EDIT_REDO, [this](wl::params) -> LRESULT {
        controller.onEditorRedoCommand();
        return 0;
    });

    on_command(ID_EDIT_CUT, [this](wl::params) -> LRESULT {
        controller.onEditorCutCommand();
        return 0;
    });

    on_command(ID_EDIT_COPY, [this](wl::params) -> LRESULT {
        controller.onEditorCopyCommand();
        return 0;
    });

    on_command(ID_EDIT_PASTE, [this](wl::params) -> LRESULT {
        controller.onEditorPasteCommand();
        return 0;
    });

    on_command(ID_HELP_ABOUT, [this](wl::params) -> LRESULT {
        MessageBox(hwnd(), _T("Xenoide"), _T("About"), MB_OK | MB_ICONINFORMATION);
        return 0;
    });

    on_command(ID_EDIT_FIND, [this](wl::params) -> LRESULT { return 0; });

    on_command(ID_EDIT_REPLACE, [this](wl::params) -> LRESULT {
        MessageBox(hwnd(), _T("Xenoide"), _T("About"), MB_OK | MB_ICONINFORMATION);
        return 0;
    });

    on_notify(IDC_EDITOR1_PAGE, SCN_MODIFIED, [this](wl::params) -> LRESULT {
        controller.onEditorModified();
        return 0;
    });

    on_notify(IDC_EDITOR1_PAGE, SCN_CHARADDED, [this](wl::params p) -> LRESULT {
        auto const &notification = *reinterpret_cast<SCNotification *>(p.lParam);
        controller.onEditorCharAdded(notification);
        return 0;
    });

    commandDataMap = {
        {ID_FILE_NEW, {_T("&New"), _T("Create a new file"), FVIRTKEY | FCONTROL, 'N'}},
        {ID_FILE_OPEN, {_T("&Open ..."), _T("Open an existing file"), FVIRTKEY | FCONTROL, 'O'}},
        {ID_FILE_SAVE, {_T("&Save"), _T("Save the active document"), FVIRTKEY | FCONTROL, 'S'}},
        {ID_FILE_SAVEAS, {_T("S&ave As ..."), _T("Save the active document under a new name"), 0, 0}},
        {ID_FILE_EXIT, {_T("&Exit"), _T("Exit Xenoide"), 0, 0}},

        {ID_EDIT_UNDO, {_T("&Undo"), _T("Undo the last action"), FVIRTKEY | FCONTROL, 'Z'}},
        {ID_EDIT_REDO, {_T("&Redo"), _T("Redo the last undone action"), FVIRTKEY | FCONTROL | FSHIFT, 'Z'}},
        {ID_EDIT_CUT, {_T("&Cut"), _T("Cut the selection to the clipboard"), FVIRTKEY | FCONTROL, 'X'}},
        {ID_EDIT_COPY, {_T("C&opy"), _T("Copy the selection to the clipboard"), FVIRTKEY | FCONTROL, 'C'}},
        {ID_EDIT_PASTE, {_T("&Paste"), _T("Paste the clipboard contents"), FVIRTKEY | FCONTROL, 'V'}},
        {ID_EDIT_FIND, {_T("&Find ..."), _T("Find text at current editor"), FCONTROL, 'F'}},
        {ID_EDIT_REPLACE, {_T("&Replace ..."), _T("Repalce text at current editor"), FCONTROL, 'H'}},

        {ID_HELP_ABOUT, {_T("&About"), _T("Display version and license information"), 0, 0}},
    };
}

void MainWindow::fillFileMenu(wl::menu &menu) {
    appendMenuItem(menu, ID_FILE_NEW);
    menu.append_separator();
    appendMenuItem(menu, ID_FILE_OPEN);
    menu.append_separator();
    appendMenuItem(menu, ID_FILE_SAVE);
    appendMenuItem(menu, ID_FILE_SAVEAS);
    menu.append_separator();
    appendMenuItem(menu, ID_FILE_EXIT);
}

void MainWindow::fillEditMenu(wl::menu &menu) {
    appendMenuItem(menu, ID_EDIT_UNDO);
    appendMenuItem(menu, ID_EDIT_REDO);
    menu.append_separator();
    appendMenuItem(menu, ID_EDIT_CUT);
    appendMenuItem(menu, ID_EDIT_COPY);
    appendMenuItem(menu, ID_EDIT_PASTE);
    menu.append_separator();
    appendMenuItem(menu, ID_EDIT_FIND);
    appendMenuItem(menu, ID_EDIT_REPLACE);
}

void MainWindow::fillHelpMenu(wl::menu &menu) {
    appendMenuItem(menu, ID_HELP_ABOUT);
}

void MainWindow::buildMenu() {
    mainMenu = wl::menu{CreateMenu()};
    fileMenu = mainMenu.append_submenu(_T("&File"));
    fillFileMenu(fileMenu);
    editMenu = mainMenu.append_submenu(_T("&Edit"));
    fillEditMenu(editMenu);
    helpMenu = mainMenu.append_submenu(_T("&Help"));
    fillHelpMenu(helpMenu);

    SetMenu(hwnd(), mainMenu.hmenu());
}

void MainWindow::buildMenuToolbar() {
    filePopupMenu = wl::menu(CreatePopupMenu());
    fillFileMenu(filePopupMenu);
    editPopupMenu = wl::menu(CreatePopupMenu());
    fillEditMenu(editPopupMenu);
    helpPopupMenu = wl::menu(CreatePopupMenu());
    fillHelpMenu(helpPopupMenu);
}

void MainWindow::buildStatusBar() {
    statusbar.create(this).add_resizable_part(1).set_text(_T("Ready"), 0);
}

void MainWindow::buildUI() {
    editor.create(this, IDC_EDITOR1_PAGE, {0, 0}, {0, 0});

    controller = MainWindowController{this, SciEditor{this->editor.get_direct_pointer(), this->editor.get_direct_function()}};

    controller.getModel()->attach(this);
    controller.onEditorModified();

    wl::font::util::set_ui_on_children(hwnd());
}

void MainWindow::layout() {
    if (!editor.hwnd())
        return;

    RECT clientRc{};
    GetClientRect(hwnd(), &clientRc);

    int statusH = 0;
    if (statusbar.hwnd()) {
        RECT sbRc{};
        GetWindowRect(statusbar.hwnd(), &sbRc);
        statusH = sbRc.bottom - sbRc.top;
    }

    int toolbarHeight = 0;

    if (hwndRebar) {
        RECT rect;
        GetWindowRect(hwndRebar, &rect);
        toolbarHeight = rect.bottom - rect.top;
    }

    int dockW = clientRc.right;
    int dockH = clientRc.bottom - statusH - toolbarHeight;
    if (dockH < 0) {
        dockH = 0;
    }

    SetWindowPos(editor.hwnd(), HWND_BOTTOM, 0, toolbarHeight, dockW, dockH, SWP_NOACTIVATE);
}

void MainWindow::buildAcceleratorTable() {
    std::vector<ACCEL> const accels = makeAcceleratorArray();

    LPACCEL const data = const_cast<LPACCEL>(accels.data());
    auto const size = static_cast<int>(accels.size());

    setup.accelTable = CreateAcceleratorTable(data, size);
}

HIMAGELIST g_hImageList = nullptr;

HWND CreateMenuToolbar(HWND hWndParent) {
    // Create the toolbar.
    DWORD const style = WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER;

    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, nullptr, style, 0, 0, 0, 0, hWndParent, (HMENU)IDC_TB_MENU, GetModuleHandle(nullptr), nullptr);

    if (hWndToolbar == nullptr) {
        return nullptr;
    }

    // configure the toolbar
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM) nullptr);

    // add buttons
    const DWORD buttonStyles = BTNS_AUTOSIZE | BTNS_DROPDOWN;
    const int numButtons = 3;
    TBBUTTON tbButtons[numButtons] = {
        {I_IMAGENONE, ID_FILE, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"&File"},
        {I_IMAGENONE, ID_EDIT, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Edit"},
        {I_IMAGENONE, ID_HELP, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Help"}
    };
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

    // Force a button height close to a real menu bar's.
    int menuHeight = GetSystemMetrics(SM_CYMENU);
    SendMessage(hWndToolbar, TB_SETBUTTONSIZE, 0, MAKELPARAM(0, menuHeight));

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}

void MainWindow::buildRebar() {
    hwndRebar = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        REBARCLASSNAME,
        nullptr,
        WS_VISIBLE | WS_CHILD | WS_BORDER | RBS_VARHEIGHT | RBS_BANDBORDERS | CCS_NODIVIDER,
        0,
        0,
        0,
        0,
        hwnd(),
        (HMENU)IDC_REBAR,
        GetModuleHandle(nullptr),
        nullptr
    );
}

void MainWindow::buildActionToolbar() {
    const int ImageListID = 0;

    toolbarImageList.create({48, 48}, ILC_MASK | ILC_COLORDDB, 3, 0);
    toolbarImageList.load_from_resource(IDI_FILE_NEW);
    toolbarImageList.load_from_resource(IDI_FILE_OPEN);
    toolbarImageList.load_from_resource(IDI_FILE_SAVE);

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    hToolBar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        nullptr,
        WS_CHILD | TBSTYLE_WRAPABLE |

            // styles required for the rebar?
            CCS_NORESIZE | CCS_NOPARENTALIGN,
        0,
        0,
        0,
        0,
        hwnd(),
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (hToolBar == nullptr) {
        return;
    }

    // Set the image list.
    SendMessage(hToolBar, TB_SETIMAGELIST, (WPARAM)ImageListID, (LPARAM)toolbarImageList.himagelist());

    // Load the button images.
    SendMessage(hToolBar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);

    // Initialize button info.
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.
    TBBUTTON tbButtons[3] = {
        {MAKELONG(0, ImageListID), ID_FILE_NEW, TBSTATE_ENABLED, buttonStyles, {0}, 0, 0},
        {MAKELONG(1, ImageListID), ID_FILE_OPEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, 0},
        {
            MAKELONG(2, ImageListID),
            ID_FILE_SAVE,
            TBSTATE_ENABLED,
            buttonStyles,
            {0},
            0,
        }
    };

    // Add buttons.
    SendMessage(hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hToolBar, TB_ADDBUTTONS, (WPARAM)3, (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hToolBar, TRUE);
}

void MainWindow::buildToolbars() {
    hToolBarMenu = CreateMenuToolbar(hwndRebar);

    buildActionToolbar();

    addRebarBand(IDC_TB_MENU, hToolBarMenu, false);
    addRebarBand(IDC_TB, hToolBar, true);
}

void MainWindow::addRebarBand(UINT wId, HWND hChild, bool breakBand) {
    SIZE sz;
    SendMessage(hChild, TB_AUTOSIZE, 0, 0);
    SendMessage(hChild, TB_GETMAXSIZE, 0, (LPARAM)&sz);

    REBARBANDINFO rbb = {0};
    rbb.cbSize = sizeof(REBARBANDINFO);
    rbb.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_ID;
    rbb.fStyle = RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE | (breakBand ? RBBS_BREAK : 0L);
    rbb.hwndChild = hChild;
    rbb.cxMinChild = 0;
    rbb.cyMinChild = sz.cy;
    rbb.cx = sz.cx;
    rbb.wID = wId;

    SendMessage(hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbb);
}

void MainWindow::update(const MainWindowNotification &notification) {
    const MainWindowModel *model = controller.getModel();

    if (notification.filePathChanged || notification.modifiedFlagChanged) {
        wl::tstring const appTitle = _T("Xenoide");
        wl::tstring const editorTitle = to_tstring(model->getEditorTitle());

        SetWindowText(this->hwnd(), (appTitle + _T(" - ") + editorTitle).c_str());
    }

    if (notification.undoBufferChanged) {
        bool const canUndo = model->getEditor().canUndo();
        bool const canRedo = model->getEditor().canRedo();

        sptr_t const canPaste = IsClipboardFormatAvailable(CF_UNICODETEXT);

        EnableMenuItem(editMenu.hmenu(), ID_EDIT_UNDO, canUndo ? MF_ENABLED : MF_DISABLED);
        EnableMenuItem(editMenu.hmenu(), ID_EDIT_REDO, canRedo ? MF_ENABLED : MF_DISABLED);
        EnableMenuItem(editMenu.hmenu(), ID_EDIT_PASTE, canPaste != 0 ? MF_ENABLED : MF_DISABLED);
    }
}

std::optional<std::string> MainWindow::showFileDialog(ShowFileDialog dialog, const ShowFileDialogOptions &options) {
    bool selected = false;

    wl::tstring filter = to_tstring(options.filter);
    wl::tstring defaultPath = to_tstring(options.defaultFile);
    wl::tstring path;

    switch (dialog) {
    case ShowFileDialog::Open:
        selected = wl::sysdlg::open_file(this->hwnd(), filter.c_str(), path);
        break;

    case ShowFileDialog::Save:
        selected = wl::sysdlg::save_file(this->hwnd(), filter.c_str(), path, defaultPath);
        break;
    }

    if (!selected) {
        return std::nullopt;
    }

    return narrow(path);
}

ShowMessageDialogButton MainWindow::showMessageDialog(const ShowMessageDialogOptions &options) {
    UINT const flags = MB_ICONQUESTION | MB_YESNOCANCEL;
    int const result = MessageBox(this->hwnd(), to_tstring(options.prompt).c_str(), to_tstring(options.title).c_str(), flags);

    switch (result) {
    case IDYES:
        return ShowMessageDialogButton::Yes;

    case IDNO:
        return ShowMessageDialogButton::No;

    case IDCANCEL:
        return ShowMessageDialogButton::Cancel;

    default:
        return ShowMessageDialogButton::Cancel;
    }
}

void MainWindow::postQuitMessage() {
    PostQuitMessage(0);
}

std::vector<ACCEL> MainWindow::makeAcceleratorArray() {
    std::vector<ACCEL> accels;

    for (const auto &pair : commandDataMap) {
        ACCEL accel = {};

        accel.cmd = pair.first;
        accel.fVirt = pair.second.fVirt;
        accel.key = pair.second.key;

        accels.emplace_back(accel);
    }

    return accels;
}

const wl::tstring MainWindow::lookupMenuHint(WORD cmdId) noexcept {
    if (auto const it = commandDataMap.find(cmdId); it != commandDataMap.end()) {
        return it->second.hint;
    }

    return _T("");
}

void MainWindow::appendMenuItem(wl::menu &menu, WORD cmd) {
    auto const it = commandDataMap.find(cmd);
    assert(it != commandDataMap.end());

    wl::tstring caption = it->second.caption;

    if (it->second.key > 0) {
        caption += _T("\t");
        caption += makeAcceleratorCaption(it->second.fVirt, it->second.key);
    }

    menu.append_item(cmd, caption);
}
