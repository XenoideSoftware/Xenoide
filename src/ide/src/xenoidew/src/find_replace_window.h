
#pragma once 


#include "winlamb/window_control.h"
#include "winlamb/resizer.h"
#include "winlamb/textbox.h"
#include "winlamb/button.h"
#include "winlamb/label.h"
#include "winlamb/checkbox.h"
#include "winlamb/resizer.h"

class FindReplaceWindow : public wl::window_control {
public:
	FindReplaceWindow();

private:
	wl::resizer _resz;
    wl::label    _lblSearch;
    wl::textbox  _txtSearch;
    wl::label    _lblReplace;
    wl::textbox  _txtReplace;
    wl::checkbox _chkCase;
    wl::checkbox _chkWord;
    wl::button   _btnFindNext;
    wl::button   _btnReplace;
    wl::button   _btnReplaceAll;
    wl::button   _btnCancel;
};
