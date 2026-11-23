
#ifndef __FELIDE_GTK3_EDITORHEADER_HPP__
#define __FELIDE_GTK3_EDITORHEADER_HPP__

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include <string>

namespace Xenoide {
    class DocumentGtk;
    class DocumentHeader : public Gtk::HBox {
    public:
        DocumentHeader(DocumentGtk &editor, const std::string &title);

        ~DocumentHeader();

        void update_title_label();

        void set_title(const std::string &title);

        Gtk::Button* get_close_button();

    private:
        DocumentGtk &m_editor;
        std::string m_title;
        Gtk::Image m_closeImage;
        Gtk::Label m_titleLabel;
        Gtk::Button m_closeButton;
    };
}

#endif
