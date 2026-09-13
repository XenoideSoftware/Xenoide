
#include "FileUtil.hpp"

#include <fstream>

namespace felide {
    std::string FileUtil::load(const std::string &file) {
        typedef std::istreambuf_iterator<char> fstream_iterator;

        std::fstream fs;
        
        fs.open(file.c_str(), std::ios_base::in);
        if (!fs.is_open()) {
            throw std::runtime_error("Couldn't open the file '" + file + "'.");
        }

        std::string content;
        content.assign(fstream_iterator(fs), fstream_iterator());

        return content;
    }

    void FileUtil::save(const std::string &file, const std::string &content) {
        std::fstream fs;
        
        fs.open(file.c_str(), std::ios_base::out);
        if (!fs.is_open()) {
            throw std::runtime_error("The file " + file + "could't be opened");
        }
        
        if (content.size() > 0) {
            fs.write(content.c_str(), content.size() - 1);
        } else {
            fs.write("", 1);
        }
    }
}
