#include "FolderExplorerPanel.h"
#include <spdlog/spdlog.h>

namespace xenoide {

    class FileItemData : public wxTreeItemData {
    public:
        FileItemData(const std::filesystem::path &path) : mPath(path) {
        }
        std::filesystem::path mPath;
    };

    FolderExplorerPanel::FolderExplorerPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        constexpr long style = wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT;
        mTreeCtrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(mTreeCtrl, 1, wxEXPAND);
        SetSizer(sizer);

        mTreeCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, &FolderExplorerPanel::OnTreeItemActivated, this);
    }

    void FolderExplorerPanel::setFolder(const std::filesystem::path &path) {
        spdlog::info("FolderExplorer: Setting folder to {}", path.string());
        mCurrentFolder = path;
        populateTree(path);
    }

    void FolderExplorerPanel::setFileActivatedCallback(FileActivatedCallback callback) {
        mFileActivatedCallback = std::move(callback);
    }

    void FolderExplorerPanel::populateTree(const std::filesystem::path &path) {
        mTreeCtrl->DeleteAllItems();
        wxTreeItemId root = mTreeCtrl->AddRoot(path.filename().string());
        addFileToTree(path, root);
        mTreeCtrl->Expand(root);
    }

    void FolderExplorerPanel::addFileToTree(const std::filesystem::path &path, wxTreeItemId parentId) {
        try {
            for (const auto &entry : std::filesystem::directory_iterator(path)) {
                wxTreeItemId id = mTreeCtrl->AppendItem(parentId, entry.path().filename().string(), -1, -1, new FileItemData(entry.path()));

                if (entry.is_directory()) {
                    addFileToTree(entry.path(), id);
                }
            }
        } catch (const std::exception &e) {
            spdlog::error("FolderExplorer: Error reading directory {}: {}", path.string(), e.what());
        }
    }

    void FolderExplorerPanel::OnTreeItemActivated(wxTreeEvent &event) {
        wxTreeItemId itemId = event.GetItem();
        if (!itemId.IsOk()) {
            return;
        }

        FileItemData *itemData = dynamic_cast<FileItemData *>(mTreeCtrl->GetItemData(itemId));
        if (itemData) {
            std::filesystem::path path = itemData->mPath;
            if (std::filesystem::is_regular_file(path)) {
                spdlog::info("FolderExplorer: File activated: {}", path.string());
                if (mFileActivatedCallback) {
                    mFileActivatedCallback(path);
                }
            }
        }
    }

} // namespace xenoide
