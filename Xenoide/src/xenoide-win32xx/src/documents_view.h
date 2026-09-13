#pragma once

#include <wxx_wincore.h>
#include <wxx_docking.h>
#include "tab_page_richtext.h"

class DocumentsView : public CDockContainer {
public:
    DocumentsView();
    ~DocumentsView() override = default;

private:
    DocumentsView(const DocumentsView&) = delete;
    DocumentsView& operator=(const DocumentsView&) = delete;

    RichEditView m_editor;
};
