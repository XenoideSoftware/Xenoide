#pragma once

#include <map>
#include <string>
#include <vector>

#include "cmcheck/Config.h"
#include "cmcheck/Finding.h"
#include "cmcheck/Lexer.h"
#include "cmcheck/Model.h"
#include "cmcheck/Severity.h"

namespace cmcheck {

    struct RuleContext {
        ProjectModel model;
        std::map<std::string, std::vector<Token>> file_tokens;
        std::map<std::string, std::string> file_contents;
        Config config;
    };

    struct RuleDefinition {
        std::string id;
        std::string name;
        Severity default_severity;
    };

    std::vector<RuleDefinition> allRuleDefinitions();

    void markTestTargets(ProjectModel &model);

    void markDefinitionLocations(ProjectModel &model);

    class RuleEngine {
    public:
        std::vector<Finding> run(const RuleContext &context) const;
    };

} // namespace cmcheck