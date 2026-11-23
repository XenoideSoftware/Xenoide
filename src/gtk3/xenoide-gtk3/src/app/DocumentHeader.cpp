
#include "DocumentHeader.h"
#include "DocumentGtk.h"

namespace Xenoide {
    DocumentHeader::DocumentHeader(DocumentGtk &editor, const std::string &title) 
            : m_editor(editor), m_closeImage(Gtk::Stock::CLOSE, Gtk::IconSize(Gtk::ICON_SIZE_MENU)) {
        m_title = title;

        update_title_label();
        
        m_closeButton.set_image(m_closeImage);
        m_closeButton.set_relief(Gtk::RELIEF_NONE);

        pack_start(m_titleLabel, true, 0);
        pack_end(m_closeButton);
        show_all();
    }

    DocumentHeader::~DocumentHeader() {}

    void DocumentHeader::update_title_label() {
        std::string title = m_title;
        if (m_editor.get_dirty_flag()) {
            title += " *";
        }

        m_titleLabel.set_text(title);
    }

    void DocumentHeader::set_title(const std::string &title) {
        m_title = title;

        update_title_label();
    }

    Gtk::Button* DocumentHeader::get_close_button() {
        return &m_closeButton;
    }
}
