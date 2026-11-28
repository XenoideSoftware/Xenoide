
#include <QApplication>

#include "MainWindowQt.h"
#include <filesystem>

#include <xenoide/ui/IDEFrameModel.h>
#include <xenoide/ui/IDEFramePresenter.h>

#include "ActionBusQt.h"

int main(int argc, char **argv) {
    using namespace Xenoide;

    QApplication app(argc, argv);

    ActionBusQt actionBus;

    auto model = IDEFrameModel();
    auto presenter = IDEFramePresenter{&actionBus, &model};
    auto view = MainWindowQt(); // {&actionBus, &presenter};

    view.show();

    // TODO: Add command line parsing
    if (argc > 1) {
        const auto fullPath = std::filesystem::current_path() / argv[1];
        presenter.openFolder(fullPath.string());
    }

    return app.exec();
}
