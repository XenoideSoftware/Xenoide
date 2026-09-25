#include "OutputLogPanel.h"
#include "WxTextCtrlSink.h"
#include <spdlog/spdlog.h>

namespace xenoide {

    OutputLogPanel::OutputLogPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        mNotebook = new wxNotebook(this, wxID_ANY);

        constexpr long textStyle = wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL;
        mOutputLog = new wxTextCtrl(mNotebook, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle);
        mAppLog = new wxTextCtrl(mNotebook, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle | wxTE_RICH);

        mNotebook->AddPage(mOutputLog, "Output");
        mNotebook->AddPage(mAppLog, "Xenoide Logs");

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(mNotebook, 1, wxEXPAND);
        SetSizer(sizer);
    }

    void OutputLogPanel::setupLogging() {
        auto wxSink = std::make_shared<WxTextCtrlSinkMt>(mAppLog);
        spdlog::default_logger()->sinks().push_back(wxSink);
    }

    void OutputLogPanel::appendOutput(const wxString &text) {
        mOutputLog->AppendText(text);
    }

    void OutputLogPanel::appendLog(const wxString &text) {
        mAppLog->AppendText(text);
    }

    void OutputLogPanel::clearOutput() {
        mOutputLog->Clear();
    }

    void OutputLogPanel::clearLog() {
        mAppLog->Clear();
    }

} // namespace xenoide
