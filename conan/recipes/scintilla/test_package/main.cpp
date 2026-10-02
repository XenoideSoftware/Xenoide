#include <vector>
#include <string>
#include <iostream>

#include <QApplication>
#include <QtPlugin>
#include <QStyleFactory>
#include <QWidget>
#include <QPushButton>
#include <QMessageBox>

#include "hello-window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    HelloWindow window;
    window.show();

    return 0;
}
