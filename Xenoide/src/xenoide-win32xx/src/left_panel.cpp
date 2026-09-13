#include "left_panel.h"

void FilesView::OnAttach() {
    DWORD style = GetStyle();
    style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
    SetStyle(style);

    DeleteAllItems();

    HTREEITEM project = InsertItem(L"Project", TVI_ROOT, TVI_LAST);
    InsertItem(L"src",     project, TVI_LAST);
    InsertItem(L"include", project, TVI_LAST);
    InsertItem(L"docs",    project, TVI_LAST);

    HTREEITEM deps = InsertItem(L"Dependencies", TVI_ROOT, TVI_LAST);
    InsertItem(L"win32xx", deps, TVI_LAST);
    InsertItem(L"mctrl",   deps, TVI_LAST);
    InsertItem(L"HexCtrl", deps, TVI_LAST);

    Expand(project, TVE_EXPAND);
    Expand(deps,    TVE_EXPAND);
}

void OutlineView::OnAttach() {
    DWORD style = GetStyle();
    style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
    SetStyle(style);

    DeleteAllItems();

    HTREEITEM symbols = InsertItem(L"Symbols", TVI_ROOT, TVI_LAST);
    InsertItem(L"main()",       symbols, TVI_LAST);
    InsertItem(L"MainFrame",    symbols, TVI_LAST);
    InsertItem(L"DocumentsView", symbols, TVI_LAST);

    Expand(symbols, TVE_EXPAND);
}

FilesContainer::FilesContainer() {
    SetTabText(L"Files");
    SetDockCaption(L"Files - Explorer");
    SetView(m_view);
}

OutlineContainer::OutlineContainer() {
    SetTabText(L"Outline");
    SetDockCaption(L"Outline - Explorer");
    SetView(m_view);
}

FilesTabDocker::FilesTabDocker() {
    SetView(m_container);
    SetBarWidth(6);
}

OutlineTabDocker::OutlineTabDocker() {
    SetView(m_container);
    SetBarWidth(6);
}
