#include <wx/wx.h>
#include <wx/xrc/xmlres.h>
#include "MainWindow.h"

class XenoideApp : public wxApp {
public:
    virtual bool OnInit() {
        wxXmlResource::Get()->InitAllHandlers();
        if (!wxXmlResource::Get()->Load("resources.xrc")) {
            wxMessageBox("Failed to load resources.xrc", "Error", wxOK | wxICON_ERROR);
            return false;
        }

        xenoide::MainWindow *mainWindow = new xenoide::MainWindow();
        mainWindow->SetTitle("Xenoide");
        mainWindow->SetSize(800, 600);
        mainWindow->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(XenoideApp);
