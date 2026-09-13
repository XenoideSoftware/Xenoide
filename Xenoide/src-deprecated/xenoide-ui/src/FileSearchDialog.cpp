
#include <xenoide/ui/FileSearchDialog.h>

#include <iostream>
#include <cassert>


namespace xenoide {
    FileSearchDialog::FileSearchDialog(FileSearchDialogPresenter *presenter) {
        this->presenter = presenter;
    }

    FileSearchDialog::~FileSearchDialog() {}
}

