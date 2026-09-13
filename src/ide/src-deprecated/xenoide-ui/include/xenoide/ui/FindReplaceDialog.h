
#pragma once 

#ifndef __XENOIDE_UI_SEARCHREPLACE_DIALOG_HPP__
#define __XENOIDE_UI_SEARCHREPLACE_DIALOG_HPP__

#include <string>
#include <optional>

namespace xenoide {
    class Document;
    class FindReplaceDialogPresenter;
    class FindReplaceDialog {
    public:
        enum SearchScope {
            Selection = 0,
            CurrentDocument = 1
        };

        struct ViewData {
            std::string title;
            std::string findWhat;
            std::optional<std::string> replaceWith;

            bool matchCase = false;
            bool matchWholeWorld = false;

            SearchScope scope = CurrentDocument;
        };

        virtual ~FindReplaceDialog();

        virtual void attachPresenter(FindReplaceDialogPresenter *presenter) = 0;

        virtual void show(const ViewData &viewData) = 0;

        virtual void hide() = 0;

        virtual void toggleReplaceControls(const bool status) = 0;
    };
}

#endif
