#pragma once

#include <wx/wx.h>
#include <wx/htmllbox.h>
#include <vector>
#include <string>

namespace xenoide {

    class FileSearchDialog : public wxDialog {
    public:
        FileSearchDialog(wxWindow *parent);
        virtual ~FileSearchDialog();

        wxString getSelectedFile() const;

    private:
        void onSearchTextInput(wxCommandEvent &event);
        void onListActivated(wxCommandEvent &event);

        wxTextCtrl *mSearchText = nullptr;
        wxSimpleHtmlListBox *mResultList = nullptr;

        std::vector<std::string> mMockFiles;
        wxString mSelectedFile;
    };

} // namespace xenoide
