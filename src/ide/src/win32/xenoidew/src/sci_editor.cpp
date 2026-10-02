
#include "sci_editor.h"

#include <cassert>
#include <cmath>

sptr_t SciEditor::send(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
    assert(scintillaPtr);
    assert(scintillaFn);

    return scintillaFn(scintillaPtr, iMessage, wParam, lParam);
}

sptr_t SciEditor::send(unsigned int iMessage, uptr_t wParam, sptr_t lParam) const {
    assert(scintillaPtr);
    assert(scintillaFn);

    return scintillaFn(scintillaPtr, iMessage, wParam, lParam);
}

SciEditor::SciEditor(sptr_t scintillaPtr, SciFnDirect scintillaFn) : scintillaPtr(scintillaPtr), scintillaFn(scintillaFn) {
    const char *fontName = "Consolas";
    int const fontSize = 10;

    send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>(fontName));

    send(SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
    send(SCI_STYLECLEARALL);
    send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    send(SCI_SETCARETLINEVISIBLE, 1);
    send(SCI_SETCARETLINEBACK, makeRGB(245, 245, 245));

    send(SCI_SETTABWIDTH, 4);
    send(SCI_SETINDENT, 4);
    send(SCI_SETUSETABS);
}

void SciEditor::syncLexerConfig(const std::optional<SciLexerConfig> &config) {
    if (config) {
        send(SCI_SETLEXER, config->lexer);
        send(SCI_SETKEYWORDS, 0, (sptr_t)config->keywords.c_str());
        for (const LanguageStyle style : config->styles) {
            send(SCI_STYLESETFORE, style.style, style.colour);
        }
    } else {
        send(SCI_SETLEXER, 0);
    }
}

void SciEditor::syncMarginLineNumber(int marginIndex, std::optional<int> lineCountOpt) {
    int lineCount = lineCountOpt.value_or(send(SCI_GETLINECOUNT));
    auto const charCount = static_cast<int>(std::log10(lineCount) + 1);

    std::string str = "_";
    for (int i = 0; i < charCount; i++) {
        str += "9";
    }

    auto const lParam = reinterpret_cast<sptr_t>(str.c_str());
    sptr_t const marginWidth = send(SCI_TEXTWIDTH, STYLE_LINENUMBER, lParam);
    send(SCI_SETMARGINWIDTHN, marginIndex, marginWidth);
}

void SciEditor::clearState() {
    send(SCI_EMPTYUNDOBUFFER);
}

void SciEditor::clearAll() {
    send(SCI_CLEARALL);
}

std::string SciEditor::getText() const {
    std::string text;
    text.resize(send(SCI_GETTEXTLENGTH) + 1);

    send(SCI_GETTEXT, (uptr_t)text.size(), (sptr_t)text.c_str());

    return text;
}

void SciEditor::setText(const std::string &text) {
    send(SCI_SETTEXT, 0, sptr_t(text.c_str()));
}

void SciEditor::setSavePoint() {
    send(SCI_SETSAVEPOINT);
}

void SciEditor::undo() {
    send(SCI_UNDO);
}

void SciEditor::redo() {
    send(SCI_REDO);
}

void SciEditor::cut() {
    send(SCI_CUT);
}

void SciEditor::copy() {
    send(SCI_COPY);
}

void SciEditor::paste() {
    send(SCI_PASTE);
}

bool SciEditor::getModify() const {
    return send(SCI_GETMODIFY) != 0;
}

bool SciEditor::canUndo() const {
    return send(SCI_CANUNDO) != 0;
}

bool SciEditor::canRedo() const {
    return send(SCI_CANREDO) != 0;
}

void SciEditor::markUndoPoint() {
    send(SCI_BEGINUNDOACTION);
    send(SCI_ENDUNDOACTION);
}
