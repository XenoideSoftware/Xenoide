#pragma once

#include <string>
#include <vector>

namespace cmcheck {

    struct TraceCommand {
        std::string file;
        int line = 0;
        int line_end = 0;
        std::string cmd;
        std::vector<std::string> args;
    };

    struct TargetInfo {
        std::string name;
        std::string type;
        std::string source_dir;
        std::string definition_file;
        int definition_line = 0;
        bool is_alias = false;
        bool is_test = false;
    };

    struct FolderInfo {
        std::string path;
        std::vector<std::string> targets;
    };

    struct ProjectModel {
        std::string root;
        std::string build_dir;
        std::vector<TargetInfo> targets;
        std::vector<FolderInfo> folders;
        std::vector<std::string> listfiles;
        std::vector<TraceCommand> commands;
    };

} // namespace cmcheck