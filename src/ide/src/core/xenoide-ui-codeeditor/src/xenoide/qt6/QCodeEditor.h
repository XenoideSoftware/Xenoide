#pragma once

#include <QWidget>
#include <string>

#include <xenoide/CodeEditor.h>

class ScintillaEdit;

namespace Scintilla {
    class ILexer5;
}

namespace xenoide::qt6 {
    class QCodeEditor : public QWidget, public xenoide::CodeEditor {
        Q_OBJECT

    public:
        explicit QCodeEditor(QWidget *parent = nullptr);

        void setContent(const std::string &content) override;

        [[nodiscard]]
        std::string getContent() const override;

        void setTitle(const std::string &title) override;

        [[nodiscard]]
        std::string getTitle() const override;

        void setLexer(Scintilla::ILexer5 *lexer);
        void setKeywords(int set, const char *keywords);
        void setStyleForeground(int style, int r, int g, int b);

    signals:
        void contentModified();
        void titleChanged(const QString &title);

    private:
        void setupTheme();

        ScintillaEdit *editor = nullptr;
        std::string title;
    };
} // namespace xenoide::qt6
