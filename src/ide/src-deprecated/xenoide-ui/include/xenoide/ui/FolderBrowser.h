
#ifndef __XENOIDE_UI_FOLDEREXPLORERVIEW_HPP_
#define __XENOIDE_UI_FOLDEREXPLORERVIEW_HPP_

#include <string>
#include <optional>
#include <filesystem>
#include <xenoide/core/Predef.h>
#include <xenoide/ui/Menu.h>

namespace xenoide {
class FolderBrowser;
class DialogManager;
class FolderService;
class FolderBrowserPresenter;

    struct Point { int x, y; };

    class FolderBrowser {
    public:
        explicit FolderBrowser(FolderBrowserPresenter *presenter);
        virtual ~FolderBrowser();

        virtual void displayFolder(const std::string &folder) = 0;

        virtual std::optional<std::string> getSelectedPath() const = 0;

        virtual void displayContextualMenu(const Point &point, const MenuData &menu) = 0;

    protected:
        FolderBrowserPresenter *presenter;
    };
} 

#endif
