
#include <QApplication>

#include <xenoide/qt6/QFolderBrowser.h>
#include <xenoide/FolderBrowserPresenter.h>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    xenoide::FileService fileService;
    xenoide::FolderBrowserPresenter presenter(gsl_lite::make_not_null(&fileService));
    xenoide::qt6::QFolderBrowser browser(gsl_lite::make_not_null(&presenter));
    presenter.openFolder("CMakeFiles");

    browser.show();

    return app.exec();
}
