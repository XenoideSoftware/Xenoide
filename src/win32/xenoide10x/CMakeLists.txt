set(target Xenoide10X)

set(sources
    main.cpp
)

add_executable(${target} WIN32 ${sources})
target_compile_definitions(${target} PUBLIC "WINVER=0x0410")
