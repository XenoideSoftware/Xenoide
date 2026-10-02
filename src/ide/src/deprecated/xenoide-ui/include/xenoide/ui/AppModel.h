
#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

namespace Xenoide {
    enum DocumentFlags {
        DF_NONE = 0x0000,
        DF_MODIFIED = 0x0001,
    };

    struct Document2 {
        std::string hash;
        std::optional<std::filesystem::path> filePath;
        DocumentFlags flags = DF_NONE;

        [[nodiscard]]
        std::string computeTitle() const {
            std::string title = "Untitled";

            if (filePath.has_value()) {
                title = filePath.value().filename();
            }

            const std::string prefix = flags & DF_MODIFIED ? " * " : "";

            return title + prefix;
        }

        [[nodiscard]]
        std::string computeFileTitle() const {
            if (filePath.has_value()) {
                return filePath.value().filename();
            }

            return "Untitled";
        }
    };

    class Workspace {
    public:
        explicit Workspace(const std::string rootPath) {
            this->rootPath = rootPath;
        }

        std::string createDocument(std::optional<std::string> filePath = {}) {
            const std::string hash = generateHash();
            documents.insert({hash, {hash, filePath, DF_NONE}});
            return hash;
        }

        void removeDocument(const std::string &hash) {
            documents.erase(hash);
        }

        Document2 getDocument(const std::string &hash) {
            return documents.at(hash);
        }

        std::string getRootPath() const {
            return rootPath;
        }

    private:
        //! documents currently opened by the user
        std::map<std::string, Document2> documents;

        // currently openened folder
        std::string rootPath;

        std::string generateHash() {
            static int counter = 0;
            return std::to_string(++counter);
        }
    };
} // namespace Xenoide
