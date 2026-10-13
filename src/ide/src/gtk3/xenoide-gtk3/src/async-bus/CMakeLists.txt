
set (target "async-bus")
set (sources 
    # "lambda-event-bus.cpp" 
    "Component.h" 
    "Component.cpp" 
    "message-bus.h" 
    "message-bus.cpp" 
    "async-bus-main.cpp")

add_executable(${target} ${sources})
