#pragma once
#include <winlamb/window_main.h>
#include <winlamb/menu.h>
#include <winlamb/statusbar.h>

class MainWindow : public wl::window_main {
public:
    MainWindow();

private:
    wl::menu      mainMenu;
    wl::statusbar statusbar;

    void buildMenu();
    void onCommand(WORD id);
};
