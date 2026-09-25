#include <catch2/catch_test_macros.hpp>

#include <string>

#include <cmcheck/TraceParser.h>

using namespace cmcheck;

TEST_CASE("TraceParser parses newline-delimited trace events", "[trace]") {
    const std::string trace =
        R"({"version":{"major":1,"minor":2}}
{"args":["VERSION","3.16"],"cmd":"cmake_minimum_required","file":"/proj/CMakeLists.txt","line":1,"time":1.0}
{"args":["demo","main.cpp"],"cmd":"add_executable","file":"/proj/CMakeLists.txt","line":4,"time":1.0}
{"args":["demo","PRIVATE","foo::bar","PUBLIC","baz"],"cmd":"target_link_libraries","file":"/proj/CMakeLists.txt","line":5,"time":1.0}
)";

    const auto commands = TraceParser().parse(trace);

    REQUIRE(commands.size() == 3);
    CHECK(commands[0].cmd == "cmake_minimum_required");
    CHECK(commands[0].args == std::vector<std::string>{"VERSION", "3.16"});
    CHECK(commands[0].file == "/proj/CMakeLists.txt");
    CHECK(commands[0].line == 1);

    CHECK(commands[1].cmd == "add_executable");
    CHECK(commands[1].args == std::vector<std::string>{"demo", "main.cpp"});

    CHECK(commands[2].cmd == "target_link_libraries");
    CHECK(commands[2].args == std::vector<std::string>{"demo", "PRIVATE", "foo::bar", "PUBLIC", "baz"});
}

TEST_CASE("TraceParser parses an array-wrapped trace", "[trace]") {
    const std::string trace =
        R"([
{"args":["VERSION","3.16"],"cmd":"cmake_minimum_required","file":"/proj/CMakeLists.txt","line":1,"time":1.0},
{"args":["demo"],"cmd":"project","file":"/proj/CMakeLists.txt","line":2,"time":1.0}
])";

    const auto commands = TraceParser().parse(trace);

    REQUIRE(commands.size() == 2);
    CHECK(commands[0].cmd == "cmake_minimum_required");
    CHECK(commands[1].cmd == "project");
}

TEST_CASE("TraceParser captures line_end for multi-line commands", "[trace]") {
    const std::string trace =
        R"({"args":["a","b","c"],"cmd":"target_link_libraries","file":"/proj/CMakeLists.txt","line":6,"line_end":9,"time":1.0}
)";

    const auto commands = TraceParser().parse(trace);

    REQUIRE(commands.size() == 1);
    CHECK(commands[0].line == 6);
    CHECK(commands[0].line_end == 9);
}

TEST_CASE("TraceParser ignores malformed lines", "[trace]") {
    const std::string trace = "not-json\n{\"args\":[\"x\"],\"cmd\":\"set\",\"file\":\"/f\",\"line\":1,\"time\":1.0}\n";
    const auto commands = TraceParser().parse(trace);
    REQUIRE(commands.size() == 1);
    CHECK(commands[0].cmd == "set");
}

TEST_CASE("TraceParser returns empty for empty input", "[trace]") {
    CHECK(TraceParser().parse("").empty());
}