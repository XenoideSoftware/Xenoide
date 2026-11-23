
set (target "async-bus")
set (sources 
    # "lambda-event-bus.cpp" 
    "Component.h" 
    "Component.cpp" 
    "MessageBus.h" 
    "MessageBus.cpp" 
    "async-bus-main.cpp")

add_executable(${target} ${sources})
