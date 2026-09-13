#include <wxx_appcore.h>
#include <wxx_appcore0.h>

#include "main_window.h"

class XenoideApp : public CWinApp {
public:
    XenoideApp() = default;
    ~XenoideApp() override = default;

protected:
    BOOL InitInstance() override {
        m_frame.Create();
        return TRUE;
    }

private:
    XenoideApp(const XenoideApp&) = delete;
    XenoideApp& operator=(const XenoideApp&) = delete;

    MainFrame m_frame;
};

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    try {
        XenoideApp app;
        return app.Run();
    }
    catch (const CException& e) {
        CString msg1;
        msg1 << e.GetText() << L'\n' << e.GetErrorString();
        CString msg2;
        msg2 << "Error: " << e.what();
        ::MessageBox(nullptr, msg1, msg2, MB_ICONERROR);
    }
    catch (const std::exception& e) {
        CString msg1 = e.what();
        ::MessageBox(nullptr, msg1, L"Error: std::exception", MB_ICONERROR);
    }
    return -1;
}
