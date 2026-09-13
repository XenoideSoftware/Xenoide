
#include "WorkspaceView.hpp"

#include <QBoxLayout>

namespace felide { namespace view { namespace qt5 {

    WorkspaceView::WorkspaceView(QWidget *parent, Workspace *workspace) : QWidget(parent) {
        m_fileModel = new QFileSystemModel();
        
        m_treeView = new QTreeView(this);
        m_treeView->setModel(m_fileModel);
        
        auto layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
        
        layout->addWidget(m_treeView);
        
        this->setLayout(layout);
        
        this->setWorkspace(workspace);        
    }
    
    void WorkspaceView::setWorkspace(Workspace *workspace) {
        m_workspace = workspace;
        
        m_fileModel->setRootPath(m_workspace->getPath().c_str());
    }
}}}
