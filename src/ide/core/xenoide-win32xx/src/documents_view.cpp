#include "documents_view.h"

DocumentsView::DocumentsView() {
    SetTabText(L"Untitled");
    SetDockCaption(L"Documents");
    SetHideSingleTab(TRUE);
    SetView(m_editor);
}
