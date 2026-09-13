
#include <xenoide/ui/FolderBrowser.h>


#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include <xenoide/core/FolderService.h>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Menu.h>
#include <xenoide/ui/IDEFrame.h>
#include <xenoide/ui/FolderBrowserModel.h>


namespace Xenoide {
    FolderBrowser::FolderBrowser(FolderBrowserPresenter *presenter) {
        presenter = presenter;
    }

    FolderBrowser::~FolderBrowser() {}
}
