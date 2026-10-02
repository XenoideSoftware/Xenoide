#include "ProjectLoader.h"

#include <algorithm>

namespace xe::cmake::io {

    std::optional<xe::cmake::core::ConcreteSyntaxTree> ProjectLoader::load_listfile(std::string_view path) const {
        std::string content;
        if (!filesystem_.read_file(path, content)) {
            return std::nullopt;
        }
        return parser_.parse_listfile(path, content);
    }

    std::vector<xe::cmake::core::ConcreteSyntaxTree> ProjectLoader::load_project(std::string_view directory) const {
        std::vector<xe::cmake::core::ConcreteSyntaxTree> trees;
        std::vector<std::string> listfiles = parser_.discover_listfiles(directory);
        std::sort(listfiles.begin(), listfiles.end());
        for (const std::string &path : listfiles) {
            std::string content;
            if (!filesystem_.read_file(path, content)) {
                continue;
            }
            trees.push_back(parser_.parse_listfile(path, content));
        }
        return trees;
    }

} // namespace xe::cmake::io