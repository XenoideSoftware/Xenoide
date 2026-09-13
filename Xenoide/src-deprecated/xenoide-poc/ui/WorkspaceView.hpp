
#pragma once

#ifndef QWORKSPACEWIDGET_HPP
#define QWORKSPACEWIDGET_HPP

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>

#include "felide/Workspace.hpp"

namespace felide { namespace view { namespace qt5 {

    class WorkspaceView : public QWidget {
        Q_OBJECT
    public:
        WorkspaceView(QWidget *parent = nullptr, Workspace *m_workspace = nullptr);
        
        void setWorkspace(Workspace *m_workspace);
        
        Workspace* getWorkspace() {
            return m_workspace;
        }

        const Workspace* getWorkspace() const {
            return m_workspace;
        }
        
    signals:
        
    public slots:
        
    private:
        Workspace *m_workspace = nullptr;
        QTreeView *m_treeView = nullptr;
        QFileSystemModel *m_fileModel = nullptr;
    };
}}}

#endif // QWORKSPACEWIDGET_HPP
