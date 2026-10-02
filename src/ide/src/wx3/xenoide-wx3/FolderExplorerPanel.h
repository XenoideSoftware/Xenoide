#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <filesystem>
#include <functional>

namespace xenoide {

    using FileActivatedCallback = std::function<void(const std::filesystem::path &)>;

    class FolderExplorerPanel : public wxPanel {
    public:
        FolderExplorerPanel(wxWindow *parent);

        void setFolder(const std::filesystem::path &path);
        const std::filesystem::path &getCurrentFolder() const {
            return mCurrentFolder;
        }

        void setFileActivatedCallback(FileActivatedCallback callback);

        wxTreeCtrl *getTreeCtrl() {
            return mTreeCtrl;
        }

    private:
        void populateTree(const std::filesystem::path &path);
        void addFileToTree(const std::filesystem::path &path, wxTreeItemId parentId);

        void OnTreeItemActivated(wxTreeEvent &event);

        wxTreeCtrl *mTreeCtrl = nullptr;
        std::filesystem::path mCurrentFolder;
        FileActivatedCallback mFileActivatedCallback;
    };

} // namespace xenoide