#include "tab_page_richtext.h"

void RichEditView::OnAttach() {
    m_font.CreatePointFont(110, L"Consolas");
    m_font = DpiScaleFont(m_font, 11);
    SetFont(m_font);

    if (m_body.IsEmpty()) {
        SetWindowText(L"Welcome to Xenoide (Win32++ sandbox)\r\n"
                      L"=====================================\r\n\r\n"
                      L"This is a CRichEdit control inside a CDockContainer.\r\n");
    } else {
        SetWindowText(m_body);
    }
}

void RichEditView::SetBody(LPCWSTR text) {
    m_body = text;
    if (IsWindow())
        SetWindowText(m_body);
}

OutputContainer::OutputContainer() {
    SetTabText(L"Output");
    SetDockCaption(L"Output");
    m_view.SetBody(L"Build output:\r\n[ok] Configure step complete.\r\n");
    SetView(m_view);
}

LogsContainer::LogsContainer() {
    SetTabText(L"Logs");
    SetDockCaption(L"Logs");
    m_view.SetBody(L"Logs:\r\n[info] Frame initialized.\r\n");
    SetView(m_view);
}

OutputTabDocker::OutputTabDocker() {
    SetView(m_container);
    SetBarWidth(6);
}

LogsTabDocker::LogsTabDocker() {
    SetView(m_container);
    SetBarWidth(6);
}
