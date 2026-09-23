
#pragma once

#include <memory>
#include <string>
#include <optional>
#include <functional>
#include <set>
#include <cassert>
#include <vector>

#include <xenoide/ui/Menu.h>

namespace xenoide {
    struct TreeItemId {
        TreeItemId() {
        }

        explicit TreeItemId(const int value) : value(value) {
        }

        bool operator==(const TreeItemId &item) const {
            return value == item.value;
        }

        bool operator!=(const TreeItemId &item) const {
            return value != item.value;
        }

        bool operator<(const TreeItemId &item) const {
            return value < item.value;
        }

        const int value = 0;
    };

    class TreeManagerController {
    public:
        virtual ~TreeManagerController() {
        }

        //! gets the contextual menu data model from the specified TreeItem.
        virtual std::vector<MenuData> getItemPopupMenuData(const TreeItemId itemId) const = 0;

        //
        virtual void clicked(const TreeItemId itemId) = 0;

        //! the number of children for a given tree item
        virtual int getChildCount(const TreeItemId itemId) const = 0;

        //! get the unique treeitem ID for a parent and its i-th child
        virtual TreeItemId getChildId(const TreeItemId itemId, const int i) const = 0;

        //! get the caption text from a given item
        virtual std::string getItemCaption(const TreeItemId itemId) const = 0;

        //! get the image text from a given item
        virtual int getItemImage(const TreeItemId itemId) const = 0;

        //! compares two items, for sorting operations in the treeview
        virtual int compare(const TreeItemId &item1, const TreeItemId &item2) const = 0;
    };
} // namespace xenoide
