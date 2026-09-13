#include "MainWindow.h"
#include "LanguageConfig.h"
#include <wx/artprov.h>

namespace xenoide {

    MainWindow::MainWindow() : wxFrame(nullptr, wxID_ANY, "Xenoide") {
        // Create the menu bar first (good practice in wx)
        createMenuBar();
        createToolBar();
        CreateStatusBar();
        SetStatusText("Ready");

        // Create the editor
        mScintilla = new wxStyledTextCtrl(this, wxID_ANY);

        setupEditor(mScintilla);

        // wxFrame with a single child automatically resizes the child to fill the client area.
        // so no explicit sizer needed unless we add more widgets.
    }

    void MainWindow::createMenuBar() {
        wxMenuBar* menuBar = new wxMenuBar;

        // File Menu
        wxMenu* fileMenu = new wxMenu;
        fileMenu->Append(wxID_NEW, "&New\tCtrl+N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Open an existing file");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save the current file");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Quit the application");
        menuBar->Append(fileMenu, "&File");

        // Edit Menu
        wxMenu* editMenu = new wxMenu;
        editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X", "Cut selection to clipboard");
        editMenu->Append(wxID_COPY, "&Copy\tCtrl+C", "Copy selection to clipboard");
        editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V", "Paste from clipboard");
        menuBar->Append(editMenu, "&Edit");

        // Help Menu
        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About", "Show info about this application");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);
        
        // Connect generic events if needed, but for now we just create the UI as requested.
        // The original code didn't seem to implement slots for these actions yet.
        Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
            Close(true);
        }, wxID_EXIT);

        Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
            wxMessageBox("Xenoide v0.1.0", "About Xenoide", wxOK | wxICON_INFORMATION);
        }, wxID_ABOUT);
    }

    void MainWindow::createToolBar() {
        wxToolBar* toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL);
        
        toolBar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR), "New file");
        toolBar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR), "Open file");
        toolBar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR), "Save file");
        
        toolBar->AddSeparator();
        
        toolBar->AddTool(wxID_CUT, "Cut", wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR), "Cut");
        toolBar->AddTool(wxID_COPY, "Copy", wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR), "Copy");
        toolBar->AddTool(wxID_PASTE, "Paste", wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR), "Paste");
        
        toolBar->Realize();
    }

    void MainWindow::setLanguage(const LanguageKeywords &keywords) {
        mScintilla->StyleClearAll();
        
        // wxStyledTextCtrl uses 0, 1 etc for keyword sets
        mScintilla->SetKeyWords(0, keywords.keywords);
        mScintilla->SetKeyWords(1, keywords.reservedKeywords);

        /*
        // Colors - example conversion if needed later
        mScintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 255, 0));
        mScintilla->StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 255));
        */
    }

    void MainWindow::setupEditor(wxStyledTextCtrl *scintilla) {
        scintilla->SetBufferedDraw(false);
        
        // Set font
        wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
        
        scintilla->SetCaretLineVisible(true);
        // scintilla->SetCaretLineBackground(wxColour(240, 240, 240));
        
        scintilla->SetIndent(4);
        scintilla->SetUseTabs(false);

        // Lexer setup
        // Uses SCLEX_CPP by default if we want C++ highlighting, 
        // but the original code seemed to manually set keywords on a default lexer or CPP lexer.
        // We'll set it to CPP to enable keyword highlighting support.
        scintilla->SetLexer(wxSTC_LEX_CPP);

        // Apply default to all styles
        scintilla->StyleClearAll();

        setLanguage(languageMap.at(LanguageDialect::GLSL_450));
    }
}
