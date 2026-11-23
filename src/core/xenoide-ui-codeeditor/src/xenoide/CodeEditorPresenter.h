#pragma once

#include <filesystem>
#include <string>

#include <gsl-lite/gsl-lite.hpp>

namespace xenoide {
    class FileService;
    class CodeEditor;

    class CodeEditorPresenter {
    public:
        explicit CodeEditorPresenter(const gsl_lite::not_null<FileService *> &fileService);

        void onInitialized(const gsl_lite::not_null<CodeEditor *> &view);

        void onNew();

        void onContentChanged();

        void onSave();

        void onSaveAs(const std::filesystem::path &filePath);

        void openFile(const std::filesystem::path &filePath);

        [[nodiscard]]
        bool isModified() const;

        [[nodiscard]]
        bool hasFilePath() const;

        [[nodiscard]]
        std::filesystem::path getFilePath() const;

    private:
        [[nodiscard]]
        std::string computeTitle() const;

        gsl_lite::not_null<FileService *> fileService;
        CodeEditor *view = nullptr;
        std::filesystem::path filePath;
        bool modified = false;
    };
} // namespace xenoide
