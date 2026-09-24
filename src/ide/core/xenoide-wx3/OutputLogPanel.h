#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>

namespace xenoide {

    class OutputLogPanel : public wxPanel {
    public:
        OutputLogPanel(wxWindow *parent);

        void setupLogging();

        void appendOutput(const wxString &text);
        void appendLog(const wxString &text);

        void clearOutput();
        void clearLog();

    private:
        wxNotebook *mNotebook = nullptr;
        wxTextCtrl *mOutputLog = nullptr;
        wxTextCtrl *mAppLog = nullptr;
    };

} // namespace xenoide
