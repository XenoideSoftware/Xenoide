#pragma once

#include <string>

namespace xenoide {
    class CodeEditor {
    public:
        virtual ~CodeEditor() = default;

        virtual void setContent(const std::string &content) = 0;

        [[nodiscard]]
        virtual std::string getContent() const = 0;

        virtual void setTitle(const std::string &title) = 0;

        [[nodiscard]]
        virtual std::string getTitle() const = 0;
    };
} // namespace xenoide
