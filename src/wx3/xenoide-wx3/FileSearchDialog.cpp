#include "FileSearchDialog.h"
#include <wx/xrc/xmlres.h>
#include <algorithm>

namespace xenoide {

    FileSearchDialog::FileSearchDialog(wxWindow *parent) {
        wxXmlResource::Get()->LoadDialog(this, parent, "FileSearchDialog");

        // Mock data
        mMockFiles = {
            "main.cpp",
            "MainWindow.h",
            "MainWindow.cpp",
            "CMakeLists.txt",
            "FileSearchDialog.h",
            "FileSearchDialog.cpp",
            "FindReplaceDialog.h",
            "FindReplaceDialog.cpp",
            "LanguageConfig.h",
            "LanguageConfig.cpp",
            "utils/StringUtil.cpp",
            "core/Engine.h"
        };

        mSearchText = XRCCTRL(*this, "txtSearch", wxTextCtrl);
        mResultList = XRCCTRL(*this, "lstResults", wxSimpleHtmlListBox);

        if (mSearchText) {
            mSearchText->Bind(wxEVT_TEXT, &FileSearchDialog::onSearchTextInput, this);
            mSearchText->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent &) {
                if (mResultList && mResultList->GetCount() > 0) {
                    mResultList->SetSelection(0); // Select first
                    wxCommandEvent dummy;
                    onListActivated(dummy);
                }
            });
            mSearchText->SetFocus();
        }

        if (mResultList) {
            mResultList->Bind(wxEVT_LISTBOX_DCLICK, &FileSearchDialog::onListActivated, this);
        }

        Centre();

        // Initial populate
        wxCommandEvent dummy;
        onSearchTextInput(dummy);
    }

    FileSearchDialog::~FileSearchDialog() {
    }

    wxString FileSearchDialog::getSelectedFile() const {
        return mSelectedFile;
    }

    void FileSearchDialog::onSearchTextInput(wxCommandEvent &) {
        wxString query = mSearchText->GetValue().Lower();
        mResultList->Clear();

        for (const auto &file : mMockFiles) {
            std::string filename = file;
            wxString wFilename(filename);
            wxString wFilenameLower = wFilename.Lower();

            if (query.IsEmpty()) {
                mResultList->Append(wFilename, new wxStringClientData(wFilename));
                continue;
            }

            size_t pos = wFilenameLower.find(query);
            if (pos != wxString::npos) {
                // Highlight match
                wxString displayLabel = wFilename;

                wxString pre = displayLabel.SubString(0, pos - 1);
                wxString match = displayLabel.SubString(pos, pos + query.Length() - 1);
                wxString post = displayLabel.SubString(pos + query.Length(), displayLabel.Length());

                wxString html = wxString::Format("%s<b>%s</b>%s", pre, match, post);

                mResultList->Append(html, new wxStringClientData(wFilename));
            }
        }

        if (mResultList->GetCount() > 0) {
            mResultList->SetSelection(0);
        }
    }

    void FileSearchDialog::onListActivated(wxCommandEvent &) {
        int sel = mResultList->GetSelection();
        if (sel != wxNOT_FOUND) {
            wxStringClientData *data = static_cast<wxStringClientData *>(mResultList->GetClientObject(sel));
            if (data) {
                mSelectedFile = data->GetData();
            } else {
                mSelectedFile = mResultList->GetString(sel);
            }
            EndModal(wxID_OK);
        }
    }

} // namespace xenoide
