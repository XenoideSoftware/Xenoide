
#include "MainApplication.h"


namespace Xenoide {
    MainApplication::MainApplication(int argc, char* argv[], const char *name) 
        : Gtk::Application(argc, argv, name) {}

    Glib::RefPtr<MainApplication> MainApplication::create(int argc, char* argv[], const char *name) {
        return Glib::RefPtr<MainApplication>(new MainApplication(argc, argv, name) );
    }

    void MainApplication::on_startup() {
        Gtk::Application::on_startup();

        // application menu
        Glib::RefPtr<Gio::Menu> fileMenu = Gio::Menu::create();
        fileMenu->append("_New", "win.file_new");
        fileMenu->append("_Open File", "win.file_open");
        fileMenu->append("_Open Project", "win.file_open_project");
        fileMenu->append("_Save", "win.file_save");
        fileMenu->append("Save _As", "win.file_save_as");
        fileMenu->append("_Close", "win.file_close");
        fileMenu->append("_Exit", "win.file_exit");

        Glib::RefPtr<Gio::Menu> mainMenu = Gio::Menu::create();
        mainMenu->append_submenu("_File", fileMenu);

        set_menubar(mainMenu);

        set_accel_for_action("win.file_new", "<Ctrl>N");
        set_accel_for_action("win.file_open", "<Ctrl>O");
        set_accel_for_action("win.file_open_project", "<Ctrl><Shift>O");
        set_accel_for_action("win.file_save", "<Ctrl>S");
        set_accel_for_action("win.file_save_as", "<Ctrl><Shift>S");
        set_accel_for_action("win.file_close", "<Ctrl>W");
    }
}
