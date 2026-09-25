#include "QCodeEditor.h"

#include <ScintillaEdit.h>
#include <Scintilla.h>
#include <QVBoxLayout>

namespace xenoide::qt6 {
    namespace {
        constexpr sptr_t rgb(int r, int g, int b) {
            return r | (g << 8) | (b << 16);
        }

        constexpr sptr_t rgba(int r, int g, int b, int a) {
            return r | (g << 8) | (b << 16) | (a << 24);
        }
    } // namespace

    QCodeEditor::QCodeEditor(QWidget *parent) : QWidget(parent) {
        editor = new ScintillaEdit(this);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(editor);

        editor->setBufferedDraw(false);
        editor->setIndent(4);
        editor->setUseTabs(false);
        setupTheme();

        connect(editor, &ScintillaEdit::modified, this, [this]() { emit contentModified(); });
    }

    void QCodeEditor::setContent(const std::string &content) {
        editor->setText(content.c_str());
        editor->setSavePoint();
    }

    std::string QCodeEditor::getContent() const {
        const QByteArray text = editor->getText(editor->textLength());
        return {text.constData(), static_cast<size_t>(text.size())};
    }

    void QCodeEditor::setTitle(const std::string &newTitle) {
        title = newTitle;
        emit titleChanged(QString::fromStdString(title));
    }

    std::string QCodeEditor::getTitle() const {
        return title;
    }

    void QCodeEditor::setLexer(Scintilla::ILexer5 *lexer) {
        editor->setILexer(reinterpret_cast<sptr_t>(lexer));
    }

    void QCodeEditor::setKeywords(int set, const char *keywords) {
        editor->setKeyWords(set, keywords);
    }

    void QCodeEditor::setStyleForeground(int style, int r, int g, int b) {
        editor->styleSetFore(style, rgb(r, g, b));
    }

    void QCodeEditor::setupTheme() {
        const sptr_t bgColor = rgb(30, 30, 30);
        const sptr_t fgColor = rgb(212, 212, 212);
        const sptr_t gutterBg = rgb(37, 37, 38);
        const sptr_t gutterFg = rgb(133, 133, 133);
        const sptr_t selectionBg = rgba(38, 79, 120, 255);
        const sptr_t caretLineBg = rgba(40, 40, 40, 255);
        const sptr_t braceLightFg = rgb(220, 220, 170);
        const sptr_t braceLightBg = rgb(60, 60, 60);
        const sptr_t braceBadFg = rgb(244, 71, 71);
        const sptr_t indentGuideFg = rgb(64, 64, 64);

        editor->styleSetFont(STYLE_DEFAULT, "Menlo");
        editor->styleSetSize(STYLE_DEFAULT, 13);
        editor->styleSetFore(STYLE_DEFAULT, fgColor);
        editor->styleSetBack(STYLE_DEFAULT, bgColor);
        editor->styleClearAll();

        editor->styleSetFore(STYLE_LINENUMBER, gutterFg);
        editor->styleSetBack(STYLE_LINENUMBER, gutterBg);
        editor->setMarginTypeN(0, SC_MARGIN_NUMBER);
        editor->setMarginWidthN(0, 48);

        editor->styleSetFore(STYLE_BRACELIGHT, braceLightFg);
        editor->styleSetBack(STYLE_BRACELIGHT, braceLightBg);
        editor->styleSetBold(STYLE_BRACELIGHT, true);
        editor->styleSetFore(STYLE_BRACEBAD, braceBadFg);
        editor->styleSetBack(STYLE_BRACEBAD, bgColor);

        editor->styleSetFore(STYLE_INDENTGUIDE, indentGuideFg);
        editor->styleSetBack(STYLE_INDENTGUIDE, bgColor);
        editor->setIndentationGuides(SC_IV_LOOKBOTH);

        editor->setElementColour(SC_ELEMENT_CARET, rgba(220, 220, 220, 255));
        editor->setCaretWidth(2);

        editor->setCaretLineVisible(true);
        editor->setElementColour(SC_ELEMENT_CARET_LINE_BACK, caretLineBg);

        editor->setElementColour(SC_ELEMENT_SELECTION_BACK, selectionBg);

        editor->setFoldMarginColour(true, gutterBg);
        editor->setFoldMarginHiColour(true, gutterBg);
    }
} // namespace xenoide::qt6
