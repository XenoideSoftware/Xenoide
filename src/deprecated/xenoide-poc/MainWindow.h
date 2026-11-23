#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>

namespace xenoide {

    struct LanguageKeywords;

    class MainWindow : public wxFrame {
    public:
        MainWindow();

    private:
        void setupEditor(wxStyledTextCtrl *scintilla);

        void createMenuBar();
        void createToolBar();

        void setLanguage(const LanguageKeywords &keywords);

        wxStyledTextCtrl *mScintilla = nullptr;
    };
} // namespace xenoide
