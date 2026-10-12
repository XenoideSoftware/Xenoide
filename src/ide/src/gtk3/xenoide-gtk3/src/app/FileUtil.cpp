
#include "FileUtil.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Xenoide {
    namespace FileUtil {
        bool load(const std::string &path, std::string &content) {
            if (std::filesystem::is_directory(path)) {
                return false;
            }

            std::ifstream file(path, std::ios::in | std::ios::binary);

            if (!file.is_open()) {
                return false;
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();

            if (file.bad()) {
                return false;
            }

            content = buffer.str();

            return true;
        }


        bool save(const std::string &path, const std::string &content) {
            std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);

            if (!file.is_open()) {
                return false;
            }

            file.write(content.data(), static_cast<std::streamsize>(content.size()));
            file.flush();

            return file.good();
        }
    }
}
