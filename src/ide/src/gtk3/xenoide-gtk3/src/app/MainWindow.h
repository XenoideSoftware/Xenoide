
#ifndef __FELIDE_GTK3_MAINWINDOW_HPP__
#define __FELIDE_GTK3_MAINWINDOW_HPP__

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include <string>

#include "DocumentManagerGtk.h"
#include "FolderBrowserGtk.h"

namespace Xenoide {
    class MainWindow : public Gtk::ApplicationWindow {
    public:
        MainWindow();

        ~MainWindow();

    private:
        void on_action_file_new();

        void on_action_file_open_project();

        void on_action_file_save();

        void on_action_file_close();

        void on_action_file_open();

        void on_action_file_save_as();

        void on_action_file_exit();

        void on_item_activated(std::string path);

    private:
        int m_untitledCounter = 1;

        DocumentManagerGtk documentManager;
        FolderBrowserGtk folderBrowser;

        // AKA Splitter
        Gtk::Paned paned;
    };
}

#endif
