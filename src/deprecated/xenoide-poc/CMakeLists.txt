
set (target xenoide-poc)
set (sources main.cpp MainWindow.cpp MainWindow.h LanguageConfig.cpp LanguageConfig.h)

add_executable(${target} WIN32 ${sources})

target_link_libraries(${target} PRIVATE wxWidgets::wxWidgets)
