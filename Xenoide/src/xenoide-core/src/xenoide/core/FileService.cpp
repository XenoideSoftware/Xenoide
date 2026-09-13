
#include <xenoide/core/FileService.h>

#include <fstream>
#include <filesystem>

namespace xenoide {
    std::string FileService::load(const std::string &filePath) {
        typedef std::istreambuf_iterator<char> fstream_iterator;

        std::fstream fs;

        fs.open(filePath.c_str(), std::ios_base::in);
        if (!fs.is_open()) {
            throw std::runtime_error("Couldn't open the file '" + filePath + "'.");
        }

        fs.seekg(0);

        std::string content;
        content.assign(fstream_iterator(fs), fstream_iterator());

        return content;
    }

    void FileService::save(const std::string &filePath, const std::string &content) {
        std::fstream fs;

        fs.open(filePath.c_str(), std::ios_base::out);
        if (!fs.is_open()) {
            throw std::runtime_error("The file " + filePath + "could't be opened");
        }

        if (content.size() > 0) {
            fs.write(content.c_str(), content.size());
        } else {
            fs.write("", 1);
        }
    }

    void FileService::touch(const std::string &filePath) {
        std::ofstream os;
        os.open(filePath.c_str(), std::ios_base::out);
        os.close();
    }

    bool FileService::exists(const std::string &path) const {
        return std::filesystem::exists(std::filesystem::path(path));
    }

    void FileService::enumerateIntoVisitor(const std::string &folder, FileSystemVisitor visitor) {
        using std::filesystem::directory_iterator;
        using std::filesystem::is_directory;
        using std::filesystem::path;

        directory_iterator current{folder}, end;

        while (current != end) {
            const path currentPath = current->path();

            const bool continue_ = visitor(currentPath.string());

            if (!continue_) {
                break;
            }

            ++current;
        }
    }

    std::vector<std::string> FileService::enumerate(const std::string &folder) {
        std::vector<std::string> children;

        using std::filesystem::directory_iterator;
        using std::filesystem::is_directory;
        using std::filesystem::path;

        directory_iterator current{folder}, end;

        while (current != end) {
            const path currentPath = current->path();

            children.push_back(currentPath.string());

            ++current;
        }

        return children;
    }

    std::string FileService::extractName(const std::string &path) const {
        return std::filesystem::path(path).filename().string();
    }

    std::vector<std::string> FileService::listChildFolders(const std::string &folderPath) const {
        auto childPathVector = std::vector<std::string>{};

        auto subPathIterator = std::filesystem::directory_iterator{std::filesystem::path(folderPath)};
        auto end = std::filesystem::directory_iterator{};

        while (subPathIterator != end) {
            childPathVector.push_back(subPathIterator->path().string());

            subPathIterator++;
        }

        return childPathVector;
    }
} // namespace xenoide
