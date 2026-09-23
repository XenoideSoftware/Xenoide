#pragma once

#include <wxx_wincore.h>
#include <wxx_dockframe.h>
#include "documents_view.h"

class MainFrame : public CDockFrame {
public:
    MainFrame();
    ~MainFrame() override = default;

    HWND Create(HWND parent = nullptr) override;

protected:
    BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
    int OnCreate(CREATESTRUCT &cs) override;
    void OnInitialUpdate() override;
    void PreCreate(CREATESTRUCT &cs) override;
    LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    MainFrame(const MainFrame &) = delete;
    MainFrame &operator=(const MainFrame &) = delete;

    BOOL OnFileExit();
    BOOL OnHelpAbout();
    void LoadDefaultDockers();

    DocumentsView m_view;
};
