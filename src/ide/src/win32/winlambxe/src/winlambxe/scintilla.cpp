#include "winlambxe/scintilla.h"
#include <tchar.h>

HMODULE wlx::scintilla::_ensure_loaded() {
    // SciLexer is Scintilla+Lexers in a single library
    static HMODULE h = LoadLibraryW(L"SciLexer.dll");
    if (!h) {
        throw std::runtime_error("Failed to load SciLexer.dll");
    }

    return h;
}

wlx::scintilla::scintilla() : wl::wnd(_hWnd), base_native_ctrl_pubm(_baseNativeCtrl), base_focus_pubm(_hWnd) {
}

wlx::scintilla &wlx::scintilla::create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size) {
    return this->create(parent->hwnd(), ctrlId, pos, size);
}

wlx::scintilla &wlx::scintilla::create(HWND hParent, int ctrlId, POINT pos, SIZE size) {
    _ensure_loaded();
    this->_baseNativeCtrl.create(hParent, ctrlId, nullptr, pos, size, _T("Scintilla"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL, 0);
    // Cache direct-call entry points for the rest of this control's lifetime.
    this->_fnDirect = reinterpret_cast<SciFnDirect>(SendMessageW(this->_hWnd, SCI_GETDIRECTFUNCTION, 0, 0));
    this->_ptrDirect = static_cast<sptr_t>(SendMessageW(this->_hWnd, SCI_GETDIRECTPOINTER, 0, 0));
    return *this;
}

sptr_t wlx::scintilla::send(unsigned msg, uptr_t wp /*= 0*/, sptr_t lp /*= 0*/) const noexcept {
    if (this->_fnDirect)
        return this->_fnDirect(this->_ptrDirect, msg, wp, lp);
    return static_cast<sptr_t>(SendMessageW(this->_hWnd, msg, static_cast<WPARAM>(wp), static_cast<LPARAM>(lp)));
}

wlx::scintilla &wlx::scintilla::set_text(const std::string &text) {
    this->send(SCI_CLEARALL);
    this->send(SCI_APPENDTEXT, static_cast<uptr_t>(text.size()), reinterpret_cast<sptr_t>(text.data()));
    return *this;
}

std::string wlx::scintilla::get_text() const {
    const intptr_t len = this->send(SCI_GETLENGTH);
    if (len <= 0)
        return {};
    std::string buf(static_cast<size_t>(len) + 1, '\0');
    const intptr_t got = this->send(SCI_GETTEXT, static_cast<uptr_t>(buf.size()), reinterpret_cast<sptr_t>(buf.data()));
    buf.resize(static_cast<size_t>(got));
    return buf;
}

wlx::scintilla &wlx::scintilla::append_text(const std::string &text) {
    this->send(SCI_APPENDTEXT, static_cast<uptr_t>(text.size()), reinterpret_cast<sptr_t>(text.data()));
    return *this;
}

wlx::scintilla &wlx::scintilla::insert_text(intptr_t pos, const std::string &text) {
    this->send(SCI_INSERTTEXT, static_cast<uptr_t>(pos), reinterpret_cast<sptr_t>(text.c_str()));
    return *this;
}

wlx::scintilla &wlx::scintilla::delete_range(intptr_t pos, intptr_t length) {
    this->send(SCI_DELETERANGE, static_cast<uptr_t>(pos), static_cast<sptr_t>(length));
    return *this;
}

wlx::scintilla &wlx::scintilla::clear_all() noexcept {
    this->send(SCI_CLEARALL);
    return *this;
}

intptr_t wlx::scintilla::length() const noexcept {
    return this->send(SCI_GETLENGTH);
}

std::string wlx::scintilla::text_range(intptr_t start, intptr_t end) const {
    if (end <= start)
        return {};
    const size_t n = static_cast<size_t>(end - start);
    std::string buf(n + 1, '\0');
    Sci_TextRange tr{};
    tr.chrg.cpMin = static_cast<long>(start);
    tr.chrg.cpMax = static_cast<long>(end);
    tr.lpstrText = buf.data();
    const intptr_t got = this->send(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
    buf.resize(static_cast<size_t>(got));
    return buf;
}

int wlx::scintilla::char_at(intptr_t pos) const noexcept {
    return static_cast<int>(this->send(SCI_GETCHARAT, static_cast<uptr_t>(pos)));
}

intptr_t wlx::scintilla::line_count() const noexcept {
    return this->send(SCI_GETLINECOUNT);
}

intptr_t wlx::scintilla::line_from_position(intptr_t pos) const noexcept {
    return this->send(SCI_LINEFROMPOSITION, static_cast<uptr_t>(pos));
}

intptr_t wlx::scintilla::position_from_line(intptr_t line) const noexcept {
    return this->send(SCI_POSITIONFROMLINE, static_cast<uptr_t>(line));
}

intptr_t wlx::scintilla::line_length(intptr_t line) const noexcept {
    return this->send(SCI_LINELENGTH, static_cast<uptr_t>(line));
}

std::string wlx::scintilla::line_text(intptr_t line) const {
    const intptr_t len = this->line_length(line);
    if (len <= 0)
        return {};
    std::string buf(static_cast<size_t>(len) + 1, '\0');
    const intptr_t got = this->send(SCI_GETLINE, static_cast<uptr_t>(line), reinterpret_cast<sptr_t>(buf.data()));
    buf.resize(static_cast<size_t>(got));
    return buf;
}

wlx::scintilla &wlx::scintilla::goto_line(intptr_t line) noexcept {
    this->send(SCI_GOTOLINE, static_cast<uptr_t>(line));
    return *this;
}

wlx::scintilla &wlx::scintilla::goto_pos(intptr_t pos) noexcept {
    this->send(SCI_GOTOPOS, static_cast<uptr_t>(pos));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_selection(intptr_t anchor, intptr_t caret) noexcept {
    this->send(SCI_SETSEL, static_cast<uptr_t>(anchor), static_cast<sptr_t>(caret));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_anchor(intptr_t pos) noexcept {
    this->send(SCI_SETANCHOR, static_cast<uptr_t>(pos));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_current_pos(intptr_t pos) noexcept {
    this->send(SCI_SETCURRENTPOS, static_cast<uptr_t>(pos));
    return *this;
}

intptr_t wlx::scintilla::selection_start() const noexcept {
    return this->send(SCI_GETSELECTIONSTART);
}

intptr_t wlx::scintilla::selection_end() const noexcept {
    return this->send(SCI_GETSELECTIONEND);
}

intptr_t wlx::scintilla::current_pos() const noexcept {
    return this->send(SCI_GETCURRENTPOS);
}

intptr_t wlx::scintilla::anchor() const noexcept {
    return this->send(SCI_GETANCHOR);
}

std::string wlx::scintilla::selected_text() const {
    const intptr_t len = this->send(SCI_GETSELTEXT);
    if (len <= 0)
        return {};
    std::string buf(static_cast<size_t>(len) + 1, '\0');
    const intptr_t got = this->send(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(buf.data()));
    buf.resize(static_cast<size_t>(got));
    return buf;
}

wlx::scintilla &wlx::scintilla::select_all() noexcept {
    this->send(SCI_SELECTALL);
    return *this;
}

wlx::scintilla &wlx::scintilla::replace_selection(const std::string &text) {
    this->send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(text.c_str()));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_readonly(bool ro) noexcept {
    this->send(SCI_SETREADONLY, ro ? 1u : 0u);
    return *this;
}

bool wlx::scintilla::is_readonly() const noexcept {
    return this->send(SCI_GETREADONLY) != 0;
}

bool wlx::scintilla::is_modified() const noexcept {
    return this->send(SCI_GETMODIFY) != 0;
}

wlx::scintilla &wlx::scintilla::set_save_point() noexcept {
    this->send(SCI_SETSAVEPOINT);
    return *this;
}

bool wlx::scintilla::can_undo() const noexcept {
    return this->send(SCI_CANUNDO) != 0;
}

bool wlx::scintilla::can_redo() const noexcept {
    return this->send(SCI_CANREDO) != 0;
}

wlx::scintilla &wlx::scintilla::undo() noexcept {
    this->send(SCI_UNDO);
    return *this;
}

wlx::scintilla &wlx::scintilla::redo() noexcept {
    this->send(SCI_REDO);
    return *this;
}

wlx::scintilla &wlx::scintilla::begin_undo_action() noexcept {
    this->send(SCI_BEGINUNDOACTION);
    return *this;
}

wlx::scintilla &wlx::scintilla::end_undo_action() noexcept {
    this->send(SCI_ENDUNDOACTION);
    return *this;
}

wlx::scintilla &wlx::scintilla::empty_undo_buffer() noexcept {
    this->send(SCI_EMPTYUNDOBUFFER);
    return *this;
}

wlx::scintilla &wlx::scintilla::style_clear_all() noexcept {
    this->send(SCI_STYLECLEARALL);
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_font(int styleIndex, const char *face) {
    this->send(SCI_STYLESETFONT, static_cast<uptr_t>(styleIndex), reinterpret_cast<sptr_t>(face));
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_size(int styleIndex, int sizePt) noexcept {
    this->send(SCI_STYLESETSIZE, static_cast<uptr_t>(styleIndex), static_cast<sptr_t>(sizePt));
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_fore(int styleIndex, COLORREF colour) noexcept {
    this->send(SCI_STYLESETFORE, static_cast<uptr_t>(styleIndex), static_cast<sptr_t>(colour));
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_back(int styleIndex, COLORREF colour) noexcept {
    this->send(SCI_STYLESETBACK, static_cast<uptr_t>(styleIndex), static_cast<sptr_t>(colour));
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_bold(int styleIndex, bool on) noexcept {
    this->send(SCI_STYLESETBOLD, static_cast<uptr_t>(styleIndex), on ? 1 : 0);
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_italic(int styleIndex, bool on) noexcept {
    this->send(SCI_STYLESETITALIC, static_cast<uptr_t>(styleIndex), on ? 1 : 0);
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_underline(int styleIndex, bool on) noexcept {
    this->send(SCI_STYLESETUNDERLINE, static_cast<uptr_t>(styleIndex), on ? 1 : 0);
    return *this;
}

wlx::scintilla &wlx::scintilla::style_set_eol_filled(int styleIndex, bool on) noexcept {
    this->send(SCI_STYLESETEOLFILLED, static_cast<uptr_t>(styleIndex), on ? 1 : 0);
    return *this;
}

wlx::scintilla &wlx::scintilla::set_lexer_language(const char *name) {
    this->send(SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<sptr_t>(name));
    return *this;
}

wlx::scintilla &wlx::scintilla::colourise(intptr_t start, intptr_t end) noexcept {
    this->send(SCI_COLOURISE, static_cast<uptr_t>(start), static_cast<sptr_t>(end));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_margin_type(int margin, int type) noexcept {
    this->send(SCI_SETMARGINTYPEN, static_cast<uptr_t>(margin), static_cast<sptr_t>(type));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_margin_width(int margin, int pixels) noexcept {
    this->send(SCI_SETMARGINWIDTHN, static_cast<uptr_t>(margin), static_cast<sptr_t>(pixels));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_margin_mask(int margin, int mask) noexcept {
    this->send(SCI_SETMARGINMASKN, static_cast<uptr_t>(margin), static_cast<sptr_t>(mask));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_margin_sensitive(int margin, bool sensitive) noexcept {
    this->send(SCI_SETMARGINSENSITIVEN, static_cast<uptr_t>(margin), sensitive ? 1 : 0);
    return *this;
}

wlx::scintilla &wlx::scintilla::show_line_numbers(bool show, int width /*= 32*/) noexcept {
    if (show) {
        this->set_margin_type(0, SC_MARGIN_NUMBER);
        this->set_margin_width(0, width);
    } else {
        this->set_margin_width(0, 0);
    }
    return *this;
}

wlx::scintilla &wlx::scintilla::marker_define(int markerNum, int symbol) noexcept {
    this->send(SCI_MARKERDEFINE, static_cast<uptr_t>(markerNum), static_cast<sptr_t>(symbol));
    return *this;
}

wlx::scintilla &wlx::scintilla::marker_set_fore(int markerNum, COLORREF colour) noexcept {
    this->send(SCI_MARKERSETFORE, static_cast<uptr_t>(markerNum), static_cast<sptr_t>(colour));
    return *this;
}

wlx::scintilla &wlx::scintilla::marker_set_back(int markerNum, COLORREF colour) noexcept {
    this->send(SCI_MARKERSETBACK, static_cast<uptr_t>(markerNum), static_cast<sptr_t>(colour));
    return *this;
}

int wlx::scintilla::marker_add(intptr_t line, int markerNum) noexcept {
    return static_cast<int>(this->send(SCI_MARKERADD, static_cast<uptr_t>(line), static_cast<sptr_t>(markerNum)));
}

wlx::scintilla &wlx::scintilla::marker_delete(intptr_t line, int markerNum) noexcept {
    this->send(SCI_MARKERDELETE, static_cast<uptr_t>(line), static_cast<sptr_t>(markerNum));
    return *this;
}

wlx::scintilla &wlx::scintilla::indicator_set_style(int indicator, int indicatorStyle) noexcept {
    this->send(SCI_INDICSETSTYLE, static_cast<uptr_t>(indicator), static_cast<sptr_t>(indicatorStyle));
    return *this;
}

wlx::scintilla &wlx::scintilla::indicator_set_fore(int indicator, COLORREF colour) noexcept {
    this->send(SCI_INDICSETFORE, static_cast<uptr_t>(indicator), static_cast<sptr_t>(colour));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_current_indicator(int indicator) noexcept {
    this->send(SCI_SETINDICATORCURRENT, static_cast<uptr_t>(indicator));
    return *this;
}

wlx::scintilla &wlx::scintilla::indicator_fill_range(intptr_t pos, intptr_t length) noexcept {
    this->send(SCI_INDICATORFILLRANGE, static_cast<uptr_t>(pos), static_cast<sptr_t>(length));
    return *this;
}

wlx::scintilla &wlx::scintilla::indicator_clear_range(intptr_t pos, intptr_t length) noexcept {
    this->send(SCI_INDICATORCLEARRANGE, static_cast<uptr_t>(pos), static_cast<sptr_t>(length));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_folding_enabled(bool on) noexcept {
    if (on) {
        this->send(SCI_SETPROPERTY, reinterpret_cast<uptr_t>("fold"), reinterpret_cast<sptr_t>("1"));
        this->set_margin_type(2, SC_MARGIN_SYMBOL);
        this->set_margin_mask(2, SC_MASK_FOLDERS);
        this->set_margin_sensitive(2, true);
        this->set_margin_width(2, 16);
        this->marker_define(SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
        this->marker_define(SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
        this->marker_define(SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
        this->marker_define(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
        this->marker_define(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
        this->marker_define(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
        this->marker_define(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
        this->send(SCI_SETFOLDFLAGS, 16, 0); // draw line below contracted fold
    } else {
        this->set_margin_width(2, 0);
        this->send(SCI_SETPROPERTY, reinterpret_cast<uptr_t>("fold"), reinterpret_cast<sptr_t>("0"));
    }
    return *this;
}

wlx::scintilla &wlx::scintilla::fold_line(intptr_t line, int action) noexcept {
    this->send(SCI_FOLDLINE, static_cast<uptr_t>(line), static_cast<sptr_t>(action));
    return *this;
}

wlx::scintilla &wlx::scintilla::fold_all(int action) noexcept {
    this->send(SCI_FOLDALL, static_cast<uptr_t>(action));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_target_range(intptr_t start, intptr_t end) noexcept {
    this->send(SCI_SETTARGETRANGE, static_cast<uptr_t>(start), static_cast<sptr_t>(end));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_search_flags(int flags) noexcept {
    this->send(SCI_SETSEARCHFLAGS, static_cast<uptr_t>(flags));
    return *this;
}

intptr_t wlx::scintilla::search_in_target(const std::string &text) noexcept {
    return this->send(SCI_SEARCHINTARGET, static_cast<uptr_t>(text.size()), reinterpret_cast<sptr_t>(text.data()));
}

intptr_t wlx::scintilla::replace_target(const std::string &text) noexcept {
    return this->send(SCI_REPLACETARGET, static_cast<uptr_t>(text.size()), reinterpret_cast<sptr_t>(text.data()));
}

intptr_t wlx::scintilla::find_text(int flags, intptr_t start, intptr_t end, const std::string &text) {
    Sci_TextToFind ttf{};
    ttf.chrg.cpMin = static_cast<long>(start);
    ttf.chrg.cpMax = static_cast<long>(end);
    ttf.lpstrText = const_cast<char *>(text.c_str());
    return this->send(SCI_FINDTEXT, static_cast<uptr_t>(flags), reinterpret_cast<sptr_t>(&ttf));
}

wlx::scintilla &wlx::scintilla::set_tab_width(int width) noexcept {
    this->send(SCI_SETTABWIDTH, static_cast<uptr_t>(width));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_use_tabs(bool on) noexcept {
    this->send(SCI_SETUSETABS, on ? 1u : 0u);
    return *this;
}

wlx::scintilla &wlx::scintilla::set_indent(int width) noexcept {
    this->send(SCI_SETINDENT, static_cast<uptr_t>(width));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_eol_mode(int mode) noexcept {
    this->send(SCI_SETEOLMODE, static_cast<uptr_t>(mode));
    return *this;
}

wlx::scintilla &wlx::scintilla::convert_eols(int mode) noexcept {
    this->send(SCI_CONVERTEOLS, static_cast<uptr_t>(mode));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_view_eol(bool on) noexcept {
    this->send(SCI_SETVIEWEOL, on ? 1u : 0u);
    return *this;
}

wlx::scintilla &wlx::scintilla::set_view_whitespace(int mode) noexcept {
    this->send(SCI_SETVIEWWS, static_cast<uptr_t>(mode));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_word_wrap(int mode) noexcept {
    this->send(SCI_SETWRAPMODE, static_cast<uptr_t>(mode));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_caret_line_visible(bool on) noexcept {
    this->send(SCI_SETCARETLINEVISIBLE, on ? 1u : 0u);
    return *this;
}

wlx::scintilla &wlx::scintilla::set_caret_line_back(COLORREF c) noexcept {
    this->send(SCI_SETCARETLINEBACK, static_cast<uptr_t>(c));
    return *this;
}

wlx::scintilla &wlx::scintilla::set_zoom(int level) noexcept {
    this->send(SCI_SETZOOM, static_cast<uptr_t>(level));
    return *this;
}

wlx::scintilla &wlx::scintilla::zoom_in() noexcept {
    this->send(SCI_ZOOMIN);
    return *this;
}

wlx::scintilla &wlx::scintilla::zoom_out() noexcept {
    this->send(SCI_ZOOMOUT);
    return *this;
}

void wlx::scintilla::process_notify(const SCNotification &n) noexcept {
    switch (n.nmhdr.code) {
    case SCN_MODIFIED:
        if (this->_onModified)
            this->_onModified(n);
        return;
    case SCN_SAVEPOINTREACHED:
        if (this->_onSavePointReached)
            this->_onSavePointReached();
        return;
    case SCN_SAVEPOINTLEFT:
        if (this->_onSavePointLeft)
            this->_onSavePointLeft();
        return;
    case SCN_CHARADDED:
        if (this->_onCharAdded)
            this->_onCharAdded(n.ch);
        return;
    case SCN_UPDATEUI:
        if (this->_onUpdateUi)
            this->_onUpdateUi(n.updated);
        return;
    case SCN_MARGINCLICK:
        if (this->_onMarginClick)
            this->_onMarginClick(static_cast<intptr_t>(n.position), n.margin, n.modifiers);
        return;
    case SCN_DOUBLECLICK:
        if (this->_onDoubleClick)
            this->_onDoubleClick(static_cast<intptr_t>(n.position), static_cast<intptr_t>(n.line), n.modifiers);
        return;
    case SCN_ZOOM:
        if (this->_onZoom)
            this->_onZoom();
        return;
    case SCN_FOCUSIN:
        if (this->_onFocusIn)
            this->_onFocusIn();
        return;
    case SCN_FOCUSOUT:
        if (this->_onFocusOut)
            this->_onFocusOut();
        return;
    case SCN_KEY:
        if (this->_onKey)
            this->_onKey(n.ch);
        return;
    default:
        if (this->_onUnhandledNotify)
            this->_onUnhandledNotify(n);
        return;
    }
}

wlx::scintilla &wlx::scintilla::on_modified(std::function<void(const SCNotification &)> fn) {
    this->_onModified = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_save_point_reached(std::function<void()> fn) {
    this->_onSavePointReached = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_save_point_left(std::function<void()> fn) {
    this->_onSavePointLeft = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_char_added(std::function<void(int)> fn) {
    this->_onCharAdded = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_update_ui(std::function<void(int)> fn) {
    this->_onUpdateUi = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_margin_click(std::function<void(intptr_t, int, int)> fn) {
    this->_onMarginClick = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_double_click(std::function<void(intptr_t, intptr_t, int)> fn) {
    this->_onDoubleClick = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_zoom(std::function<void()> fn) {
    this->_onZoom = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_focus_in(std::function<void()> fn) {
    this->_onFocusIn = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_focus_out(std::function<void()> fn) {
    this->_onFocusOut = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_key(std::function<void(int)> fn) {
    this->_onKey = std::move(fn);
    return *this;
}

wlx::scintilla &wlx::scintilla::on_unhandled_notify(std::function<void(const SCNotification &)> fn) {
    this->_onUnhandledNotify = std::move(fn);
    return *this;
}
