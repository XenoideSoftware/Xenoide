find_package(nlohmann_json CONFIG REQUIRED)
find_package(ryml CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

set (target cmake-checker-core)

set (headers
    "src/cmcheck/Severity.h"
    "src/cmcheck/Model.h"
    "src/cmcheck/Finding.h"
    "src/cmcheck/Config.h"
    "src/cmcheck/Lexer.h"
    "src/cmcheck/TraceParser.h"
    "src/cmcheck/FileApiParser.h"
    "src/cmcheck/FileUtil.h"
    "src/cmcheck/Glob.h"
    "src/cmcheck/ConfigLoader.h"
    "src/cmcheck/CmakeDriver.h"
    "src/cmcheck/Rules.h"
    "src/cmcheck/Report.h"
    "src/cmcheck/Check.h"
)

set (sources
    ${headers}
    "src/cmcheck/Lexer.cpp"
    "src/cmcheck/TraceParser.cpp"
    "src/cmcheck/FileApiParser.cpp"
    "src/cmcheck/FileUtil.cpp"
    "src/cmcheck/Glob.cpp"
    "src/cmcheck/ConfigLoader.cpp"
    "src/cmcheck/CmakeDriver.cpp"
    "src/cmcheck/Rules.cpp"
    "src/cmcheck/Report.cpp"
    "src/cmcheck/Check.cpp"
)

add_library(${target} ${sources})
target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target} PUBLIC xe::interface nlohmann_json::nlohmann_json ryml::ryml fmt::fmt)