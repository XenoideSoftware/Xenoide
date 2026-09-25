#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/treectrl.h>

#include <wx/aui/aui.h>
#include <wx/aui/tabmdi.h>
#include <filesystem>
#include <vector>
#include <map>

namespace xenoide {
    struct LanguageKeywords;
    class FolderExplorerPanel;
    class OutputLogPanel;

    class MainWindow : public wxAuiMDIParentFrame {
    public:
        MainWindow();
        virtual ~MainWindow();

    private:
        void setupEditor(wxStyledTextCtrl *scintilla);

        void createMenuBar();
        void createToolBar();
        void createLayout();

        void setLanguage(wxStyledTextCtrl *editor, const LanguageKeywords &keywords);

        // Event handlers
        void OnNew(wxCommandEvent &event);
        void OnOpenFolder(wxCommandEvent &event);
        void OnFindReplace(wxCommandEvent &event);
        void OnGoToFile(wxCommandEvent &event);
        void OnUndo(wxCommandEvent &event);
        void OnRedo(wxCommandEvent &event);

        void openFile(const std::filesystem::path &path);

        void populateOutline();

        wxAuiManager m_mgr;
        wxNotebook *mLeftPanelNotebook = nullptr;
        FolderExplorerPanel *mFolderExplorer = nullptr;
        wxTreeCtrl *mOutlineTree = nullptr;

        OutputLogPanel *mOutputLogPanel = nullptr;
    };
} // namespace xenoide
