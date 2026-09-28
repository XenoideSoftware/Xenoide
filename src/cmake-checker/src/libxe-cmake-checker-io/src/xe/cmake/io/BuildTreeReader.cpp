#include "BuildTreeReader.h"

#include <nlohmann/json.hpp>

namespace xe::cmake::io {
    namespace {

        bool read_file(const IFileSystem &filesystem, std::string_view path, nlohmann::json &out) {
            std::string content;
            if (!filesystem.read_file(path, content)) {
                return false;
            }
            try {
                out = nlohmann::json::parse(content);
                return true;
            } catch (const nlohmann::json::exception &) {
                return false;
            }
        }

    } // namespace

    void BuildTreeReader::read_trace(std::string_view trace_path, xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        nlohmann::json trace;
        if (!read_file(filesystem_, trace_path, trace)) {
            return;
        }
        if (!trace.is_array()) {
            return;
        }
        for (const nlohmann::json &event : trace) {
            if (!event.contains("args")) {
                continue;
            }
            const nlohmann::json &args = event["args"];
            if (!args.is_array() || args.size() < 1) {
                continue;
            }
            const std::string command = args[0].get<std::string>();
            if (command != "target_link_libraries" || args.size() < 3) {
                continue;
            }
            const std::string source = args[1].get<std::string>();
            for (std::size_t i = 2; i < args.size(); ++i) {
                const std::string target = args[i].get<std::string>();
                if (target == "PUBLIC" || target == "PRIVATE" || target == "INTERFACE") {
                    continue;
                }
                graph.add_edge(source, target);
            }
        }
    }

    void BuildTreeReader::read_codemodel(std::string_view codemodel_dir, xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        nlohmann::json codemodel;
        const std::string path = std::string(codemodel_dir) + "/codemodel-v2.json";
        if (!read_file(filesystem_, path, codemodel)) {
            return;
        }
        if (!codemodel.contains("configurations")) {
            return;
        }
        for (const nlohmann::json &configuration : codemodel["configurations"]) {
            if (!configuration.contains("targets")) {
                continue;
            }
            for (const nlohmann::json &target : configuration["targets"]) {
                if (!target.contains("name") || !target.contains("dependencies")) {
                    continue;
                }
                const std::string target_name = target["name"].get<std::string>();
                graph.add_node(target_name);
                if (!target.contains("dependencies")) {
                    continue;
                }
                for (const nlohmann::json &dependency : target["dependencies"]) {
                    std::string dependency_name;
                    if (dependency.contains("name")) {
                        dependency_name = dependency["name"].get<std::string>();
                    } else if (dependency.contains("id")) {
                        dependency_name = dependency["id"].get<std::string>();
                    } else {
                        continue;
                    }
                    graph.add_edge(target_name, dependency_name);
                }
            }
        }
    }

} // namespace xe::cmake::io