
#ifndef __FELIDE_GTK3_EDITOR_HPP__
#define __FELIDE_GTK3_EDITOR_HPP__

#include <string>

#include <gtkmm.h>
#include <gtksourceviewmm.h>

namespace Xenoide {
    typedef sigc::signal<void> signal_editor_dirty_changed_t;

    /**
     * @brief A Simple Text DocumentGtk control with scrollbars
     */
    class DocumentGtk : public Gtk::Bin {
    public:
        DocumentGtk(const DocumentGtk&) = delete;

        DocumentGtk(const std::string &key);
        
        virtual ~DocumentGtk();

        void set_text(const std::string &text);

        std::string get_text() const;

        std::string get_key() const;

        void set_key(const std::string &key);

        bool has_file_path() const;

        void set_dirty_flag(const bool flag);

        bool get_dirty_flag() const;

        void on_text_buffer_changed();
        
    private:
        std::string m_key;
        mutable bool m_dirty_flag = false;
        Gtk::ScrolledWindow m_scrolled;
        Gsv::View m_textView;

        signal_editor_dirty_changed_t m_signal_editor_dirty_changed;

    public:
        signal_editor_dirty_changed_t signal_editor_dirty_changed();
    };
}

#endif
