
#include "MainWindow.h"

#include "FileUtil.h"

#include <cassert>
#include <iostream>

namespace fs = std::filesystem;

namespace {
    void show_error_dialog(Gtk::Window &parent, const std::string &message) {
        Gtk::MessageDialog dialog(parent, message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_title("XenoideGTK");
        dialog.run();
    }
}

namespace Xenoide {
    MainWindow::MainWindow() {
        // setup supported actions
        add_action("file_new", sigc::mem_fun(*this, &MainWindow::on_action_file_new));
        add_action("file_open", sigc::mem_fun(*this, &MainWindow::on_action_file_open));
        add_action("file_open_project", sigc::mem_fun(*this, &MainWindow::on_action_file_open_project));
        add_action("file_save", sigc::mem_fun(*this, &MainWindow::on_action_file_save));
        add_action("file_save_as", sigc::mem_fun(*this, &MainWindow::on_action_file_save_as));
        add_action("file_close", sigc::mem_fun(*this, &MainWindow::on_action_file_close));
        add_action("file_exit", sigc::mem_fun(*this, &MainWindow::on_action_file_exit));

        // setup client area
        set_border_width(10);

        paned.add1(folderBrowser);

        folderBrowser.show();

        folderBrowser.signal_item_activated().connect(sigc::mem_fun(*this, &MainWindow::on_item_activated));

        paned.add2(documentManager);
        documentManager.show();

        add(paned);
        paned.show();
       
        maximize();
    }


    MainWindow::~MainWindow() {}


    void MainWindow::on_action_file_new() {
        const std::string title = "Untitled " + std::to_string(m_untitledCounter++);

        documentManager.open_editor(title, title, "");
    }


    void MainWindow::on_action_file_open_project() {
        Gtk::FileChooserDialog dialog("Please choose a Folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Open", Gtk::RESPONSE_OK);
        dialog.set_transient_for(*this);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            const std::string folderPath = dialog.get_filename();
            folderBrowser.displayFolder(folderPath);
        }
    }


    void MainWindow::on_action_file_save() {
        auto *editor = documentManager.get_current_editor();

        if (!editor) {
            return;
        }

        // TODO: Don't always assume that a Key is a Path
        if (!editor->has_file_path()) {
            on_action_file_save_as();
            return;
        }

        const std::string path = editor->get_key();
        const std::string text = editor->get_text();

        if (!FileUtil::save(path, text)) {
            show_error_dialog(*this, "Failed to save file:\n" + path);
            return;
        }

        editor->set_dirty_flag(false);
    }


    void MainWindow::on_action_file_close() {
        auto *editor = documentManager.get_current_editor();

        if (!editor) {
            return;
        }

        documentManager.close_editor(*editor);
    }


    void MainWindow::on_action_file_open() {
        auto textFilter = Gtk::FileFilter::create();
        textFilter->set_name("Text files");
        textFilter->add_mime_type("text/plain");

        auto allFilter = Gtk::FileFilter::create();
        allFilter->set_name("All files");
        allFilter->add_pattern("*");

        Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Open", Gtk::RESPONSE_OK);
        dialog.set_transient_for(*this);
        dialog.add_filter(textFilter);
        dialog.add_filter(allFilter);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {            
            const std::string path = dialog.get_filename();
            const std::string name = fs::path(path).filename().string();
            std::string content;

            if (!FileUtil::load(path, content)) {
                show_error_dialog(*this, "Failed to load file:\n" + path);
                return;
            }

            documentManager.open_editor(path, name, content);
        }
    }


    void MainWindow::on_action_file_save_as() {
        auto *editor = documentManager.get_current_editor();

        if (!editor) {
            return;
        }

        // trigger dialog
        auto textFilter = Gtk::FileFilter::create();
        textFilter->set_name("Text files");
        textFilter->add_mime_type("text/plain");

        auto allFilter = Gtk::FileFilter::create();
        allFilter->set_name("All files");
        allFilter->add_pattern("*");

        Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Save", Gtk::RESPONSE_OK);
        dialog.set_transient_for(*this);
        dialog.add_filter(textFilter);
        dialog.add_filter(allFilter);
        dialog.set_do_overwrite_confirmation(true);

        if (editor->has_file_path()) {
            dialog.set_filename(editor->get_key());
        }

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            const std::string filePath = dialog.get_filename();

            if (!FileUtil::save(filePath, editor->get_text())) {
                show_error_dialog(*this, "Failed to save file:\n" + filePath);
                return;
            }

            const std::string name = fs::path(filePath).filename().string();

            documentManager.change_editor_key(*editor, filePath, name);
            editor->set_dirty_flag(false);
        }
    }


    void MainWindow::on_action_file_exit() {
        hide();
    }


    void MainWindow::on_item_activated(std::string path) {
        if (fs::is_directory(path)) {
            return;
        }

        const std::string name = fs::path(path).filename().string();
        std::string content;

        if (!FileUtil::load(path, content)) {
            show_error_dialog(*this, "Failed to load file:\n" + path);
            return;
        }

        documentManager.open_editor(path, name, content);
    }
}
