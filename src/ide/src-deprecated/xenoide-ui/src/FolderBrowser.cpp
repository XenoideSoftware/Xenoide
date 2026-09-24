
#include <xenoide/ui/FolderBrowser.h>

namespace xenoide {
    FolderBrowser::FolderBrowser(FolderBrowserPresenter *presenter) {
        this->presenter = presenter;
    }

    FolderBrowser::~FolderBrowser() {
    }
} // namespace xenoide
