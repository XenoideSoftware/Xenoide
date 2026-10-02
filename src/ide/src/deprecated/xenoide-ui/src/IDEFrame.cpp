
#include <xenoide/ui/IDEFrame.h>

#include <iostream>
#include <cassert>

#include <xenoide/ui/Menu.h>
#include <xenoide/ui/MenuPanel.h>
#include <xenoide/ui/FolderBrowser.h>
#include <xenoide/ui/DocumentManagerPresenter.h>
#include <xenoide/ui/DocumentManagerModel.h>

namespace xenoide {
    IDEFrame::IDEFrame(IDEFramePresenter *presenter) {
        mPresenter = presenter;
    }

    IDEFrame::~IDEFrame() {
    }
} // namespace xenoide
