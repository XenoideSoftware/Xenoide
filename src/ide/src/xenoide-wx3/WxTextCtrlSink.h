#pragma once

#include <wx/wx.h>
#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace xenoide {

    template <typename Mutex> class WxTextCtrlSink : public spdlog::sinks::base_sink<Mutex> {
    public:
        explicit WxTextCtrlSink(wxTextCtrl *textCtrl) : mTextCtrl(textCtrl) {
        }

    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
            const wxString text = wxString::FromUTF8(formatted.data(), formatted.size());

            wxColour color = getColorForLevel(msg.level);

            // Post to main thread since wxWidgets UI must be updated from main thread
            wxTextCtrl *ctrl = mTextCtrl;
            wxTheApp->CallAfter([ctrl, text, color]() {
                if (ctrl) {
                    long insertionPoint = ctrl->GetLastPosition();
                    ctrl->AppendText(text);
                    long endPoint = ctrl->GetLastPosition();

                    // Apply color to the newly added text
                    ctrl->SetStyle(insertionPoint, endPoint, wxTextAttr(color));
                }
            });
        }

        void flush_() override {
            // No buffering needed for wxTextCtrl
        }

    private:
        static wxColour getColorForLevel(spdlog::level::level_enum level) {
            switch (level) {
            case spdlog::level::trace:
            case spdlog::level::debug:
                return wxColour(128, 128, 128); // Gray
            case spdlog::level::info:
                return *wxBLACK;
            case spdlog::level::warn:
                return wxColour(204, 153, 0); // Yellow/Orange
            case spdlog::level::err:
            case spdlog::level::critical:
                return *wxRED;
            default:
                return *wxBLACK;
            }
        }

        wxTextCtrl *mTextCtrl;
    };

    using WxTextCtrlSinkMt = WxTextCtrlSink<std::mutex>;
    using WxTextCtrlSinkSt = WxTextCtrlSink<spdlog::details::null_mutex>;

} // namespace xenoide
