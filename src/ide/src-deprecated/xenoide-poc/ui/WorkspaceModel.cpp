
#include "WorkspaceModel.hpp"

#include <cassert>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace felide { namespace view { namespace qt5 {

    WorkspaceModel::WorkspaceModel(Workspace *workspace) {
        m_workspace = workspace;
    }
    
    QVariant WorkspaceModel::data(const QModelIndex &index, int role) const {
        
        if (role == Qt::DisplayRole) {
            
            if (index == QModelIndex()) {
                fs::path path = m_workspace->getPath();
                
                return QVariant(path.stem().string().c_str());
                
            } else {
                std::list<int> fileIndices;
                
                for (QModelIndex current=index; current!=QModelIndex(); current=current.parent()) {
                    fileIndices.push_front(current.row());
                }
                
                fs::path path = m_workspace->getPath();
                
                for (int fileIndex : fileIndices) {
                    auto fileIterator = fs::directory_iterator(path);
                    
                    std::advance(fileIterator, fileIndex);
                    
                    path = fileIterator->path();
                }
                
                return QVariant(path.stem().string().c_str());
            }
        } else {
            std::cerr << "WorkspaceModel::data: Invalid role " << role << std::endl;
            
            return QVariant();
        }
    }
    
    /*
    Qt::ItemFlags WorkspaceModel::flags(const QModelIndex &index) const {
        return Qt::ItemFlags::enum_type::ItemIsEnabled;
    }
    
    QVariant WorkspaceModel::headerData(int section, Qt::Orientation orientation, int role) const {
        return QVariant("");
    }
    */
    
    QModelIndex WorkspaceModel::index(int row, int column, const QModelIndex &parent) const {
        return QModelIndex();
    }
    
    QModelIndex WorkspaceModel::parent(const QModelIndex &index) const {
        return index;
    }
    
    int WorkspaceModel::rowCount(const QModelIndex &parent) const {
        if (parent == QModelIndex()) {
            
            fs::path path = m_workspace->getPath();
            
            auto fileIterator = fs::directory_iterator(path);
            
            return std::distance(fileIterator, {});
            
        } else {
            return 0;
        }
    }
    
    int WorkspaceModel::columnCount(const QModelIndex &parent) const {
        return 1;
    }
}}}
