
#pragma once

#ifndef __felide_mainwindow_hpp__
#define __felide_mainwindow_hpp__

#include <QMainWindow>
#include <memory>

namespace felide {
    class MenuBar;
    class StringResource;
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(const MenuBar &menuBar, const StringResource *stringResource);
        ~MainWindow();

    private:
        struct Private;
        std::unique_ptr<Private> m_impl;
    };
}

#endif
