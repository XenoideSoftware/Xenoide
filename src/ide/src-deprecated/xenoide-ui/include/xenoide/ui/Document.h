
#ifndef __XENOIDE_UI_DOCUMENT_HPP__
#define __XENOIDE_UI_DOCUMENT_HPP__

#include <string>
#include <memory>

#include <xenoide/core/Predef.h>

namespace xenoide {
    struct DocumentConfig {
        std::string fontName;
        int fontSize;
        int tabWidth;
        bool caretLineVisible;
        bool showLineNumbers;

        static DocumentConfig Default();
    };

    struct TextSelection {
        int start;
        int end;

        static TextSelection all() {
            return {-1, -1};
        }
    };

    class DialogManager;
    class Document {
    public:
        virtual ~Document();

        virtual void setTitle(const std::string &title) = 0;
        virtual std::string getTitle() const = 0;

        virtual void setContent(const std::string &content) = 0;
        virtual std::string getContent() const = 0;

        virtual void setConfig(const DocumentConfig &config) = 0;
        virtual DocumentConfig getConfig() const = 0;

        virtual void clearAll() = 0;

        virtual void clearUndoBuffer() = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;
        
        virtual void setSelection(const TextSelection &selection) = 0;

        virtual void selectAll() = 0;

        virtual void clearSelection() = 0;

        virtual TextSelection getSelection() const = 0;
    };    
}

#endif
