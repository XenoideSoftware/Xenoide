#include "main_window.h"
#include "left_panel.h"
#include "tab_page_richtext.h"
#include "resource.h"

#include <memory>

MainFrame::MainFrame() {
    SetView(m_view);
}

HWND MainFrame::Create(HWND parent) {
    return CDockFrame::Create(parent);
}

void MainFrame::PreCreate(CREATESTRUCT &cs) {
    CDockFrame::PreCreate(cs);
    cs.cx = 1100;
    cs.cy = 700;
}

int MainFrame::OnCreate(CREATESTRUCT &cs) {
    UseToolBar(FALSE);
    UseReBar(FALSE);
    UseIndicatorStatus(FALSE);
    return CDockFrame::OnCreate(cs);
}

void MainFrame::OnInitialUpdate() {
    LoadDefaultDockers();
    ShowWindow(GetInitValues().showCmd);
}

void MainFrame::LoadDefaultDockers() {
    const DWORD style = DS_CLIENTEDGE;

    CDocker *leftRoot = AddDockedChild(std::make_unique<FilesTabDocker>(), DS_DOCKED_LEFT | style, DpiScaleInt(260), ID_DOCK_LEFT);
    if (leftRoot) {
        leftRoot->AddDockedChild(std::make_unique<OutlineTabDocker>(), DS_DOCKED_CONTAINER | style, DpiScaleInt(260), ID_DOCK_LEFT + 1);
    }

    CDocker *bottomRoot = AddDockedChild(std::make_unique<OutputTabDocker>(), DS_DOCKED_BOTTOM | style, DpiScaleInt(200), ID_DOCK_BOTTOM);
    if (bottomRoot) {
        bottomRoot->AddDockedChild(std::make_unique<LogsTabDocker>(), DS_DOCKED_CONTAINER | style, DpiScaleInt(200), ID_DOCK_BOTTOM + 1);
    }

    SetDockStyle(style);
}

BOOL MainFrame::OnCommand(WPARAM wparam, LPARAM) {
    UINT id = LOWORD(wparam);
    switch (id) {
    case IDM_FILE_EXIT:
        return OnFileExit();
    case IDM_HELP_ABOUT:
        return OnHelpAbout();
    case IDM_FILE_NEW:
    case IDM_FILE_OPEN:
    case IDM_FILE_SAVE:
    case IDM_FILE_SAVEAS:
    case IDM_EDIT_UNDO:
    case IDM_EDIT_REDO:
    case IDM_EDIT_CUT:
    case IDM_EDIT_COPY:
    case IDM_EDIT_PASTE:
        return TRUE;
    }
    return FALSE;
}

BOOL MainFrame::OnFileExit() {
    Close();
    return TRUE;
}

BOOL MainFrame::OnHelpAbout() {
    MessageBox(L"Xenoide Win32++ Sandbox", L"About", MB_OK | MB_ICONINFORMATION);
    return TRUE;
}

LRESULT MainFrame::WndProc(UINT msg, WPARAM wparam, LPARAM lparam) {
    try {
        return WndProcDefault(msg, wparam, lparam);
    } catch (const CException &e) {
        CString msg1;
        msg1 << e.GetText() << L'\n' << e.GetErrorString();
        CString msg2;
        msg2 << "Error: " << e.what();
        ::MessageBox(nullptr, msg1, msg2, MB_ICONERROR);
    }
    return 0;
}
