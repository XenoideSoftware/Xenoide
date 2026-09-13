
#ifndef __XENOIDE_UI_FOLDEREXPLORERVIEW_HPP_
#define __XENOIDE_UI_FOLDEREXPLORERVIEW_HPP_

#include <string>
#include <boost/optional/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <xenoide/core/Predef.h>
#include <xenoide/ui/Menu.h>

namespace Xenoide {
    class FolderBrowser;
    class DialogManager;
    class FolderService;

    struct Point { int x, y; };

    class FolderBrowser {
    public:
        class Model {
        public:
            virtual ~Model();

            virtual void setCurrentFolderPath(const boost::filesystem::path &folderPath) = 0;

            virtual boost::filesystem::path getCurrentFolderPath() const = 0;

            virtual std::vector<boost::filesystem::path> listChildPaths(const boost::filesystem::path &folderPath) const = 0;

        public:
            static std::unique_ptr<Model> create(FolderService *folderService);
        };

        class Presenter {
        public:
            Presenter(Model *model);

            void onInitialized(FolderBrowser *folderBrowser, DialogManager *dialogManager);

            virtual ~Presenter();

            virtual void onBrowseFolder();

            virtual void onCreateFile();

            virtual void onCreateFolder();

            virtual void onMoveSelectedPath(const std::string &targetFolder);

            virtual void onRenameSelectedPath();

            virtual void onOpenSelectedFile();

            virtual void onDeleteSelectedPath();
        
            virtual void onDisplayFolder(const boost::filesystem::path &folderPath);

            virtual void onContextMenuRequested(const Point &point);

        private:
            boost::optional<std::string> askValidPath(const std::string &title, const std::string &prompt, const std::string &promptForInvalidInput, const std::string &defaultValue);

        private:
            DialogManager *dialogView;
            FolderBrowser *view;
            FolderBrowserModel *model;
        };


    public:
        explicit FolderBrowser(FolderBrowserPresenter *presenter);
        virtual ~FolderBrowser();

        virtual void displayFolder(const std::string &folder) = 0;

        virtual boost::optional<std::string> getSelectedPath() const = 0;

        virtual void displayContextualMenu(const Point &point, const MenuData &menu) = 0;

    protected:
        FolderBrowserPresenter *presenter;
    };
} 

#endif
