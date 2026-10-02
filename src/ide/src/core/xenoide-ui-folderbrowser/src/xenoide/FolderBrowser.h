#pragma once

#include <string>

namespace xenoide {
    /**
     * Abstract view interface for the folder browser component.
     * The presenter drives the tree entirely through this interface,
     * keeping it decoupled from any GUI framework.
     */
    class FolderBrowser {
    public:
        virtual ~FolderBrowser() = default;

        /** Remove all items and reset the tree to an empty state. */
        virtual void clearItems() = 0;

        /**
         * Add the root item representing the opened folder.
         * @param itemId   Stable integer ID assigned by the presenter.
         * @param name     Display name (folder's basename).
         * @param path     Full filesystem path (used by the view for signals).
         */
        virtual void addRootItem(int itemId, const std::string &name, const std::string &path) = 0;

        /**
         * Add a child item under an existing parent.
         * @param parentId  ID of the parent item (already added to the tree).
         * @param childId   Stable integer ID assigned by the presenter.
         * @param name      Display name (file or folder basename).
         * @param path      Full filesystem path.
         * @param isFolder  True → the item represents a directory (can be expanded).
         */
        virtual void addChildItem(int parentId, int childId, const std::string &name, const std::string &path, bool isFolder) = 0;

        /**
         * Visually expand the item identified by itemId.
         * Called by the presenter after its children have been loaded,
         * so that the expansion signal does not re-trigger lazy loading.
         */
        virtual void expandItem(int itemId) = 0;
    };
} // namespace xenoide
