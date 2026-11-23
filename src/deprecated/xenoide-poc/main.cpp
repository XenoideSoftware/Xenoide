#include <wx/wx.h>
#include "MainWindow.h"

class XenoideApp : public wxApp {
public:
    virtual bool OnInit() {
        xenoide::MainWindow *mainWindow = new xenoide::MainWindow();
        mainWindow->SetTitle("Xenoide");
        mainWindow->SetSize(800, 600);
        mainWindow->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(XenoideApp);
