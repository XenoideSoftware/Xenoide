
#include "hello-window.h"

#include <ScintillaEdit.h>
#include <QVBoxLayout>

HelloWindow::HelloWindow() {
    auto *scintilla = new ScintillaEdit(this);
    scintilla->setText("Hello, World!");
    setWindowTitle("Hello Qt");
    resize(200, 100);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(scintilla);
    setLayout(layout);
}
