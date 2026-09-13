
#pragma once

#ifndef QWORKSPACEMODEL_HPP
#define QWORKSPACEMODEL_HPP

#include <QObject>
#include <QAbstractItemModel>

#include "felide/Workspace.hpp"

namespace felide { namespace view { namespace qt5 {
    
    class WorkspaceModel : public QAbstractItemModel {
        Q_OBJECT
    public:
        explicit WorkspaceModel(Workspace *m_workspace);
        
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        
    private:
        Workspace *m_workspace = nullptr;
        
    signals:
        
    public slots:
        
    };

}}}

#endif
