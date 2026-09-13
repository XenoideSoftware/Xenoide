
#include <xenoide/ui/FileSearchDialogModel.h>

#include <iostream>
#include <cassert>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace xenoide {
    static bool is_logically_hidden(const std::filesystem::path &path) {
        const std::string title = path.filename().string();

        std::cout << path.string() << " isHidden: " << (title[0] == '.') << std::endl;

        return title[0] == '.';
    }

    class FileSearchDialogModelImpl : public FileSearchDialogModel {
    public:
        FileSearchDialogModelImpl(const std::filesystem::path &basePath) {
            this->basePath = basePath;
        }

        virtual ~FileSearchDialogModelImpl() {}

        virtual std::vector<std::filesystem::path> searchFilePattern(const std::string &filePattern, const int maxResults) override {
            std::string filePatternUppercased = filePattern;
            std::transform(filePatternUppercased.begin(), filePatternUppercased.end(), filePatternUppercased.begin(), ::toupper);

            return this->search(filePatternUppercased, maxResults);
        }

    private:
        bool testFile(const std::filesystem::path &filePath, const std::string &filePattern) const {
            std::string testAgaint = filePath.filename().string();
            std::transform(testAgaint.begin(), testAgaint.end(), testAgaint.begin(), ::toupper);

            return testAgaint.find(filePattern) != std::string::npos;
        }

        std::vector<std::filesystem::path> search(const std::string &filePattern, const int maxResults) const {
            std::vector<std::filesystem::path> files;

            this->searchImpl(files, basePath, filePattern, maxResults);

            return files;
        }

        void searchImpl(std::vector<std::filesystem::path> &files, const std::filesystem::path &folder, const std::string &filePattern, const int maxResults) const {
            // skips search inside files ...
            if (! std::filesystem::is_directory(folder)) {
                return;
            }

            // skips hidden directories
            if ( std::filesystem::is_directory(folder) && is_logically_hidden(folder)) {
                return;
            }

            std::filesystem::directory_iterator subPathIterator(folder);
            std::filesystem::directory_iterator end;

            while (subPathIterator != end) {
                std::filesystem::path subPath = subPathIterator->path();

                if (this->testFile(subPath, filePattern)) {
                    files.push_back(subPath);
                }

                if (files.size() + 1 >= maxResults) {
                    return;
                }

                this->searchImpl(files, subPath, filePattern, maxResults);

                subPathIterator++;
            }
        }

    private:
        std::filesystem::path basePath;
        std::vector<std::filesystem::path> files;
    };

    FileSearchDialogModel::~FileSearchDialogModel() {}

    std::unique_ptr<FileSearchDialogModel> FileSearchDialogModel::create(const std::filesystem::path &basePath) {
        return std::make_unique<FileSearchDialogModelImpl>(basePath);
    }
}
