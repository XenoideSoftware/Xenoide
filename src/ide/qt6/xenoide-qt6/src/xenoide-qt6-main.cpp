
#include <QApplication>

#include "QXenoideMainWindow.h"
#include <filesystem>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    auto mainWindow = xenoide::MainWindow();
    mainWindow.show();

    return app.exec();
}
