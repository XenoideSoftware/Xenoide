
#include "FileUtil.h"

#include <cassert>
#include <fstream>

namespace xe {
    std::string loadTextFile(const std::string &filePath) {
        assert(filePath != "");

        std::fstream fs;
        fs.open(filePath.c_str());

        assert(fs.is_open());

        std::string content;
        std::string line;

        while (std::getline(fs, line)) {
            content += line + "\n";
        }

        return content;
    }
} // namespace xe
