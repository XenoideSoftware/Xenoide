#include "DefaultCmakeParser.h"

#include "xe/cmake/core/Lexer.h"

#include <algorithm>
#include <functional>

namespace xe::cmake::io {

    xe::cmake::core::ConcreteSyntaxTree DefaultCmakeParser::parse_listfile(std::string_view file_path, std::string_view source) const {
        return xe::cmake::core::Lexer::parse(file_path, source);
    }

    std::vector<std::string> DefaultCmakeParser::discover_listfiles(std::string_view directory) const {
        std::vector<std::string> listfiles;

        std::function<void(const std::string &)> walk = [&](const std::string &dir) {
            for (const std::string &name : filesystem_.list_directory(dir)) {
                const std::string full = dir.empty() ? name : dir + "/" + name;
                if (filesystem_.is_directory(full)) {
                    walk(full);
                } else if (filesystem_.is_file(full)) {
                    const bool is_listfile = name == "CMakeLists.txt" || (name.size() > 5 && name.compare(name.size() - 5, 5, ".cmake") == 0);
                    if (is_listfile) {
                        listfiles.push_back(full);
                    }
                }
            }
        };

        walk(std::string(directory));
        std::sort(listfiles.begin(), listfiles.end());
        return listfiles;
    }

} // namespace xe::cmake::io