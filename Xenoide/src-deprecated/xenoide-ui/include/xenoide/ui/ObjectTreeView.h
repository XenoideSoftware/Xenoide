
#pragma once 

#include <memory>
#include <string>
#include <optional>
#include <functional>
#include <set>
#include <cassert>


namespace xenoide {
    struct TreeItemId {
        TreeItemId() {}
        
        explicit TreeItemId(const int value) : value(value) {}
        
        bool operator== (const TreeItemId& item) const {
            return value == item.value;
        }

        bool operator!= (const TreeItemId& item) const {
            return value != item.value;
        }
        
        bool operator< (const TreeItemId& item) const {
            return value < item.value;
        }

        const int value = 0;
    };
    

    class TreeDataModel {
    public:
        virtual ~TreeDataModel() {}

        //! the number of children for a given tree item
        virtual int getChildCount(const TreeItemId itemId) const = 0;

        //! get the unique treeitem ID for a parent and its i-th child
        virtual TreeItemId getChildId(const TreeItemId itemId, const int i) const = 0;

        //! get the caption text from a given item
        virtual std::string getItemCaption(const TreeItemId itemId) const = 0;

        //! get the image text from a given item
        virtual int getItemImage(const TreeItemId itemId) const = 0;
    };
}
