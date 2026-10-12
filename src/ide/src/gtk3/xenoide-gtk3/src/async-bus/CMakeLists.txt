
set (target "async-bus")
set (sources "lambda-event-bus.cpp" "message-bus.cpp")

add_executable(${target} ${sources})
