
set (target poc-wxminimal)
set (sources main.cpp)

add_executable(${target} WIN32 ${sources})

target_link_libraries(${target} PRIVATE wxWidgets::wxWidgets)
