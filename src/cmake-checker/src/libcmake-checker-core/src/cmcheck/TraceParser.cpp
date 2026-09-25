#include "cmcheck/TraceParser.h"

#include <nlohmann/json.hpp>

#include <sstream>
#include <string>

namespace cmcheck {

namespace {

bool parseEvent(const nlohmann::json &object, TraceCommand &out) {
    if (!object.is_object() || !object.contains("cmd")) {
        return false;
    }
    if (object.contains("version")) {
        return false;
    }
    out.cmd = object.value("cmd", std::string());
    out.file = object.value("file", std::string());
    out.line = object.value("line", 0);
    out.line_end = object.value("line_end", 0);
    if (object.contains("args") && object["args"].is_array()) {
        for (const auto &arg : object["args"]) {
            if (arg.is_string()) {
                out.args.push_back(arg.get<std::string>());
            } else {
                out.args.push_back(arg.dump());
            }
        }
    }
    return true;
}

} // namespace

std::vector<TraceCommand> TraceParser::parse(const std::string &traceJson) const {
    std::vector<TraceCommand> commands;

    const nlohmann::json document = nlohmann::json::parse(traceJson, nullptr, false);
    if (!document.is_discarded() && document.is_array()) {
        for (const auto &element : document) {
            TraceCommand command;
            if (parseEvent(element, command)) {
                commands.push_back(command);
            }
        }
        return commands;
    }

    std::istringstream stream(traceJson);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }
        const nlohmann::json object = nlohmann::json::parse(line, nullptr, false);
        if (object.is_discarded()) {
            continue;
        }
        TraceCommand command;
        if (parseEvent(object, command)) {
            commands.push_back(command);
        }
    }
    return commands;
}

} // namespace cmcheck