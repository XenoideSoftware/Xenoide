

#include <gtkmm.h>
#include <gtksourceviewmm.h>

#include "MainWindow.h"
#include "MainApplication.h"

using namespace Xenoide;

int main(int argc, char* argv[]) {
    auto app = MainApplication::create(argc, argv, "org.fapablazacl.XenoideGTK");

    MainWindow window;

    return app->run(window);
}
