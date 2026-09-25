#include "MainWindow.h"
#include "LanguageConfig.h"
#include "FolderExplorerPanel.h"
#include "OutputLogPanel.h"
#include <wx/artprov.h>
#include <wx/dirdlg.h>
#include "FindReplaceDialog.h"
#include "FileSearchDialog.h"

#include <wx/tokenzr.h>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

namespace xenoide {

    MainWindow::MainWindow() : wxAuiMDIParentFrame(nullptr, wxID_ANY, "Xenoide", wxDefaultPosition, wxSize(1024, 768), wxDEFAULT_FRAME_STYLE) {
        spdlog::info("MainWindow: Initializing application");

        m_mgr.SetManagedWindow(this);
        m_mgr.SetFlags(m_mgr.GetFlags() | wxAUI_MGR_LIVE_RESIZE);

        createMenuBar();
        createToolBar();
        // CreateStatusBar();
        // SetStatusText("Ready");

        createLayout();

        // Bind events
        Bind(wxEVT_MENU, &MainWindow::OnNew, this, wxID_NEW);
        Bind(wxEVT_MENU, &MainWindow::OnOpenFolder, this, wxID_OPEN);
        Bind(wxEVT_MENU, &MainWindow::OnFindReplace, this, wxID_REPLACE);

        spdlog::info("MainWindow: Initialization complete");
    }

    MainWindow::~MainWindow() {
        m_mgr.UnInit();
    }

    void MainWindow::createLayout() {
        // Create Left Notebook
        mLeftPanelNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(200, -1));

        // Create Folder Explorer
        mFolderExplorer = new FolderExplorerPanel(mLeftPanelNotebook);
        mFolderExplorer->setFileActivatedCallback([this](const std::filesystem::path &path) { openFile(path); });

        // Create Outline Tree
        mOutlineTree = new wxTreeCtrl(mLeftPanelNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);
        populateOutline();

        mLeftPanelNotebook->AddPage(mFolderExplorer, "Files");
        mLeftPanelNotebook->AddPage(mOutlineTree, "Outline");

        // Add Left Panel to AUI Manager
        m_mgr.AddPane(
            mLeftPanelNotebook,
            wxAuiPaneInfo()
                .Name("LeftPanel")
                .Caption("Project")
                .Left()
                .Layer(1)
                .Position(1)
                .CloseButton(false)
                .MaximizeButton(false)
                .MinimizeButton(false)
                .PinButton(true)
                .BestSize(250, -1)
                .MinSize(150, -1)
        );

        // Create Output Log Panel
        mOutputLogPanel = new OutputLogPanel(this);
        mOutputLogPanel->setupLogging();

        // Add Bottom Panel to AUI Manager
        m_mgr.AddPane(
            mOutputLogPanel,
            wxAuiPaneInfo()
                .Name("BottomPanel")
                .Caption("Output")
                .Bottom()
                .Layer(1)
                .Position(1)
                .CloseButton(true)
                .MaximizeButton(false)
                .MinimizeButton(true)
                .PinButton(true)
                .BestSize(-1, 150)
                .MinSize(-1, 100)
        );

        // Add the MDI Client Window as the Center Pane
        // wxAuiMDIParentFrame creates the client window automatically.
        m_mgr.AddPane(GetClientWindow(), wxAuiPaneInfo().Name("MDIClient").CenterPane().PaneBorder(false));

        m_mgr.Update();
    }

    void MainWindow::populateOutline() {
        mOutlineTree->DeleteAllItems();
        wxTreeItemId root = mOutlineTree->AddRoot("Root");

        wxTreeItemId cls = mOutlineTree->AppendItem(root, "class MainWindow");
        // mOutlineTree->SetItemImage(cls, ...); // If image list set

        mOutlineTree->AppendItem(cls, "MainWindow()");
        mOutlineTree->AppendItem(cls, "~MainWindow()");
        mOutlineTree->AppendItem(cls, "createLayout()");
        mOutlineTree->AppendItem(cls, "populateOutline()");

        // mOutlineTree->ExpandAll();
    }

    void MainWindow::createMenuBar() {
        wxMenuBar *menuBar = new wxMenuBar;

        // File Menu
        wxMenu *fileMenu = new wxMenu;
        fileMenu->Append(wxID_NEW, "&New\tCtrl+N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "Open &Folder...\tCtrl+O", "Open a folder");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save the current file");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Quit the application");
        menuBar->Append(fileMenu, "&File");

        // Edit Menu
        wxMenu *editMenu = new wxMenu;
        editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z", "Undo last action");
        editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y", "Redo last action");
        editMenu->AppendSeparator();
        editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X", "Cut selection to clipboard");
        editMenu->Append(wxID_COPY, "&Copy\tCtrl+C", "Copy selection to clipboard");
        editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V", "Paste from clipboard");
        editMenu->AppendSeparator();
        editMenu->Append(wxID_REPLACE, "Find/R&eplace...\tCtrl+H", "Find and Replace text");

        int idGoToFile = wxNewId();
        editMenu->Append(idGoToFile, "Go to &File...\tCtrl+P", "Search for a file");
        Bind(wxEVT_MENU, &MainWindow::OnGoToFile, this, idGoToFile);

        menuBar->Append(editMenu, "&Edit");

        // Help Menu
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About", "Show info about this application");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);

        Bind(wxEVT_MENU, [this](wxCommandEvent &) { Close(true); }, wxID_EXIT);

        Bind(wxEVT_MENU, [this](wxCommandEvent &) { wxMessageBox("Xenoide v0.1.0", "About Xenoide", wxOK | wxICON_INFORMATION); }, wxID_ABOUT);

        Bind(wxEVT_MENU, &MainWindow::OnUndo, this, wxID_UNDO);
        Bind(wxEVT_MENU, &MainWindow::OnRedo, this, wxID_REDO);
    }

    void MainWindow::createToolBar() {
        // NOTE: Not using the stock bitmaps here because if so the app segfaults on Ubuntu 25.10

        wxToolBar *toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT);

        toolBar->AddTool(wxID_NEW, "New", wxNullBitmap, "New file");
        toolBar->AddTool(wxID_OPEN, "Open Folder", wxNullBitmap, "Open folder");
        toolBar->AddTool(wxID_SAVE, "Save", wxNullBitmap, "Save file");

        toolBar->AddSeparator();

        toolBar->AddTool(wxID_CUT, "Cut", wxNullBitmap, "Cut");
        toolBar->AddTool(wxID_COPY, "Copy", wxNullBitmap, "Copy");
        toolBar->AddTool(wxID_PASTE, "Paste", wxNullBitmap, "Paste");

        toolBar->Realize();
    }

    void MainWindow::OnNew(wxCommandEvent &) {
        spdlog::info("User action: New file");

        wxAuiMDIChildFrame *child = new wxAuiMDIChildFrame(this, wxID_ANY, "Untitled");

        auto *editor = new wxStyledTextCtrl(child, wxID_ANY);
        setupEditor(editor);

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(editor, 1, wxEXPAND);
        child->SetSizer(sizer);

        // child->SetIcon(wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
        child->Show(true);
    }

    void MainWindow::OnOpenFolder(wxCommandEvent &) {
        // spdlog::info("User action: Open folder dialog");

        // wxDirDialog *dlg = new wxDirDialog(this, "Open Folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        wxDirDialog *dlg = new wxDirDialog(this);

        if (dlg->ShowModal() == wxID_OK) {
            const std::string folderPath = dlg->GetPath().ToStdString();
            spdlog::info("Opening folder: {}", folderPath);
            mFolderExplorer->setFolder(folderPath);
        }

        dlg->Destroy();
    }

    void MainWindow::openFile(const std::filesystem::path &path) {
        spdlog::info("Opening file: {}", path.string());

        std::ifstream t(path);
        if (!t.is_open()) {
            spdlog::error("Failed to open file: {}", path.string());
            wxMessageBox("Failed to open file: " + path.string(), "Error", wxOK | wxICON_ERROR);
            return;
        }

        std::stringstream buffer;
        buffer << t.rdbuf();

        wxAuiMDIChildFrame *child = new wxAuiMDIChildFrame(this, wxID_ANY, path.filename().string());
        child->SetIcon(wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));

        auto *editor = new wxStyledTextCtrl(child, wxID_ANY);
        setupEditor(editor);
        editor->SetText(buffer.str());
        editor->SetClientObject(new wxStringClientData(path.string()));

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(editor, 1, wxEXPAND);
        child->SetSizer(sizer);

        child->Show(true);
    }

    void MainWindow::OnFindReplace(wxCommandEvent &) {
        spdlog::info("User action: Find/Replace dialog");
        FindReplaceDialog dlg(this);
        dlg.ShowModal();
    }

    void MainWindow::OnGoToFile(wxCommandEvent &) {
        spdlog::info("User action: Go to file dialog");

        FileSearchDialog dlg(this);
        if (dlg.ShowModal() == wxID_OK) {
            wxString filename = dlg.getSelectedFile();
            if (!filename.IsEmpty()) {
                // Try to open it relative to current folder
                std::filesystem::path fullPath = mFolderExplorer->getCurrentFolder() / filename.ToStdString();

                if (std::filesystem::exists(fullPath)) {
                    openFile(fullPath);
                } else {
                    spdlog::error("File not found: {}", fullPath.string());
                    wxMessageBox("File not found in current folder: " + filename, "Error", wxOK | wxICON_ERROR);
                }
            }
        }
    }

    void MainWindow::OnUndo(wxCommandEvent &) {
        spdlog::info("User action: Undo");

        wxAuiMDIChildFrame *activeChild = GetActiveChild();
        if (activeChild) {
            wxWindow *win = activeChild->GetSizer()->GetItem((size_t)0)->GetWindow();
            if (auto *editor = dynamic_cast<wxStyledTextCtrl *>(win)) {
                editor->Undo();
            }
        }
    }

    void MainWindow::OnRedo(wxCommandEvent &) {
        spdlog::info("User action: Redo");

        wxAuiMDIChildFrame *activeChild = GetActiveChild();
        if (activeChild) {
            wxWindow *win = activeChild->GetSizer()->GetItem((size_t)0)->GetWindow();
            if (auto *editor = dynamic_cast<wxStyledTextCtrl *>(win)) {
                editor->Redo();
            }
        }
    }

    void MainWindow::setLanguage(wxStyledTextCtrl *editor, const LanguageKeywords &keywords) {
        editor->StyleClearAll();

        editor->SetKeyWords(0, keywords.keywords);
        editor->SetKeyWords(1, keywords.reservedKeywords);
    }

    void MainWindow::setupEditor(wxStyledTextCtrl *scintilla) {
        scintilla->SetBufferedDraw(false);

        wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

        scintilla->SetCaretLineVisible(true);
        scintilla->SetIndent(4);
        scintilla->SetUseTabs(false);

        scintilla->SetLexer(wxSTC_LEX_CPP);
        scintilla->StyleClearAll();

        setLanguage(scintilla, languageMap.at(LanguageDialect::GLSL_450));
    }
} // namespace xenoide
