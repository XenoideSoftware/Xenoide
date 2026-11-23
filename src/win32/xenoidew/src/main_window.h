#pragma once

#include <string>
#include <optional>
#include <map>
#include <winlamb/window_main.h>
#include <winlamb/menu.h>
#include <winlamb/statusbar.h>

#include "winlambxe/utils/dock_layout.h"
#include "winlambxe/utils/dock_window.h"
#include "winlambxe/utils/document_tabs.h"
#include "winlambxe/scintilla.h"
#include "winlambxe/toolbar.h"
#include "app_icons.h"
#include "left_panel.h"
#include "tab_page_richtext.h"
#include "main_window_controller.h"

struct CommandData {
    wl::tstring caption;
    wl::tstring hint;
    BYTE fVirt = 0;
    WORD key = 0;
};

class MainWindow : public wl::window_main, public MainWindowView {
public:
    MainWindow();

private:
    void update(const MainWindowNotification &notification) override;

    void buildMenu();
    void buildMenuToolbar();
    void buildStatusBar();
    void buildUI();
    void buildAcceleratorTable();
    void buildToolbars();
    void buildActionToolbar();

    void layout();

    void buildRebar();
    void addRebarBand(UINT wId, HWND hChild, bool breakBand = false);

    void fillFileMenu(wl::menu &menu);
    void fillEditMenu(wl::menu &menu);
    void fillHelpMenu(wl::menu &menu);

    std::optional<std::string> showFileDialog(ShowFileDialog dialog, const ShowFileDialogOptions &options) override;

    ShowMessageDialogButton showMessageDialog(const ShowMessageDialogOptions &options) override;

    void postQuitMessage() override;

    std::vector<ACCEL> makeAcceleratorArray();
    const wl::tstring lookupMenuHint(WORD cmdId) noexcept;
    void appendMenuItem(wl::menu &menu, WORD cmd);

    wl::menu mainMenu;
    wl::menu fileMenu;
    wl::menu editMenu;
    wl::menu helpMenu;

    wl::menu filePopupMenu;
    wl::menu editPopupMenu;
    wl::menu helpPopupMenu;

    wl::statusbar statusbar;

    wlx::toolbar toolbar;
    wlx::scintilla editor;

    std::map<WORD, CommandData> commandDataMap;

    MainWindowController controller;

    HWND hToolBarMenu = nullptr;
    HWND hToolBar = nullptr;
    HWND hwndRebar = nullptr;

    wl::image_list toolbarImageList;

    HMENU hFileMenu;
};
