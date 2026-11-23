
#ifndef __FELIDE_GTK3_EDITORMANAGER_HPP__
#define __FELIDE_GTK3_EDITORMANAGER_HPP__

#include <gtkmm.h>
#include <gtksourceviewmm.h>

#include "DocumentGtk.h"

namespace Xenoide {
    typedef sigc::signal<bool, DocumentGtk*> signal_editor_closed_t;

    class DocumentManagerGtk : public Gtk::Bin {
    public:
        DocumentManagerGtk();

        virtual ~DocumentManagerGtk();

    public:
        void open_editor(const std::string &key, const std::string &title, const std::string &content);

        DocumentGtk* get_current_editor();

        void change_editor_key(DocumentGtk &editor, const std::string &newKey, const std::string &newTitle);

        void close_editor(DocumentGtk &editor);

    public:
        signal_editor_closed_t signal_editor_closed();

    private:
        Gtk::Notebook m_notebook;
        std::map<std::string, DocumentGtk*> m_editors;
        signal_editor_closed_t m_signal_editor_closed;
    };
}

#endif
