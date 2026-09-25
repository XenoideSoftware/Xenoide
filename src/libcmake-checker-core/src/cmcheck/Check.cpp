#include "cmcheck/Check.h"

#include <string>
#include <vector>

#include "cmcheck/CmakeDriver.h"
#include "cmcheck/ConfigLoader.h"
#include "cmcheck/FileApiParser.h"
#include "cmcheck/FileUtil.h"
#include "cmcheck/Lexer.h"
#include "cmcheck/Report.h"
#include "cmcheck/Rules.h"
#include "cmcheck/TraceParser.h"

namespace cmcheck {

    namespace {

        bool hasProjectDeclaration(const std::string &rootCmakeLists) {
            const std::string content = readTextFile(rootCmakeLists);
            for (const std::string &line : splitLines(content)) {
                const std::string trimmed = trim(line);
                if (startsWith(trimmed, "project(") || startsWith(trimmed, "project (")) {
                    return true;
                }
            }
            return false;
        }

    } // namespace

    CheckResult runCheck(const CheckOptions &options, std::ostream &out) {
        CheckResult result;

        const std::string projectRoot = absolutePath(options.project);
        const std::string rootCmakeLists = joinPath(projectRoot, "CMakeLists.txt");
        if (!fileExists(rootCmakeLists)) {
            result.tool_error = true;
            result.error_message = "project '" + projectRoot + "' has no CMakeLists.txt";
            return result;
        }
        if (!hasProjectDeclaration(rootCmakeLists)) {
            result.tool_error = true;
            result.error_message = "project '" + projectRoot + "' root CMakeLists.txt must declare project(...)";
            return result;
        }

        const std::string buildDir = CmakeDriver::locateBuildDir(projectRoot, options.build_dir);
        if (buildDir.empty()) {
            result.tool_error = true;
            result.error_message = "unable to locate the project build tree. Run 'configure:cmake-check' first or pass "
                                   "--project-build-dir";
            return result;
        }

        const std::string replyDir = joinPath(buildDir, ".cmake/api/v1/reply");
        const std::string tracePath = joinPath(buildDir, "trace.json");
        if (!CmakeDriver::hasTrace(buildDir)) {
            result.tool_error = true;
            result.error_message = "trace.json not found in build tree '" + buildDir + "'. Run 'configure:cmake-check' first";
            return result;
        }

        Config config = discoverConfig(projectRoot, options.config_file);

        FileApiParser fileApiParser;
        const FileApiParser::Result parsed = fileApiParser.parse(replyDir);
        if (!parsed.error.empty()) {
            result.tool_error = true;
            result.error_message = parsed.error;
            return result;
        }
        ProjectModel model = parsed.model;

        TraceParser traceParser;
        std::vector<TraceCommand> allCommands = traceParser.parse(readTextFile(tracePath));
        std::vector<TraceCommand> projectCommands;
        for (const TraceCommand &command : allCommands) {
            if (command.file.empty() || !isSubPath(command.file, model.root)) {
                continue;
            }
            if (!model.build_dir.empty() && isSubPath(command.file, model.build_dir)) {
                continue;
            }
            projectCommands.push_back(command);
        }
        model.commands = projectCommands;

        markTestTargets(model);
        markDefinitionLocations(model);

        RuleContext context;
        context.model = model;
        context.config = config;

        for (const std::string &listfile : model.listfiles) {
            if (isExcluded(config, listfile)) {
                continue;
            }
            const std::string content = readTextFile(listfile);
            context.file_contents[listfile] = content;
            context.file_tokens[listfile] = Lexer().tokenize(content);
        }

        RuleEngine engine;
        result.findings = engine.run(context);

        Report report;
        report.print(result.findings, out);
        return result;
    }

} // namespace cmcheck