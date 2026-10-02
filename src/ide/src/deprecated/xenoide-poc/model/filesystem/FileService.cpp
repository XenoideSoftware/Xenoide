
#include <xenoide/core/FileService.h>

#include <fstream>

namespace Xenoide {
    std::string FileService::load(const std::filesystem::path &filePath) {
        typedef std::istreambuf_iterator<char> fstream_iterator;

        std::fstream fs;

        fs.open(filePath.string().c_str(), std::ios_base::in);
        if (!fs.is_open()) {
            throw std::runtime_error("Couldn't open the file '" + filePath.string() + "'.");
        }

        fs.seekg(0);

        std::string content;
        content.assign(fstream_iterator(fs), fstream_iterator());

        return content;
    }

    void FileService::save(const std::filesystem::path &filePath, const std::string &content) {
        std::fstream fs;

        fs.open(filePath.string().c_str(), std::ios_base::out);
        if (!fs.is_open()) {
            throw std::runtime_error("The file " + filePath.string() + "could't be opened");
        }

        if (content.size() > 0) {
            fs.write(content.c_str(), content.size());
        } else {
            fs.write("", 1);
        }
    }

    void FileService::touch(const std::filesystem::path &filePath) {
        std::ofstream os;
        os.open(filePath.string().c_str(), std::ios_base::out);
        os.close();
    }
} // namespace Xenoide
