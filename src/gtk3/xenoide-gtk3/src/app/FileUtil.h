
#ifndef __FELIDE_GTK3_FILEUTIL_HPP__
#define __FELIDE_GTK3_FILEUTIL_HPP__

#include <string>

namespace Xenoide {
    namespace FileUtil {
        bool load(const std::string &path, std::string &content);

        bool save(const std::string &path, const std::string &content);
    }
}

#endif
