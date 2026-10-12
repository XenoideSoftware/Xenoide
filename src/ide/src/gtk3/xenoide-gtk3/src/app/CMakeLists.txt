
#
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0 gtksourceviewmm-3.0 REQUIRED)

#
add_definitions(${GTK4_CFLAGS_OTHER})

# target 
set(target "XenoideGTK")

set(sources "main.cpp"
    DocumentHeader.h     
    DocumentHeader.cpp      
    DocumentManagerGtk.h  
    DocumentManagerGtk.cpp  
    DocumentGtk.h    
    DocumentGtk.cpp  
    FileUtil.h
    FileUtil.cpp
    FolderBrowserGtk.h    
    FolderBrowserGtk.cpp  
    MainApplication.h    
    MainApplication.cpp  
    MainWindow.h
    MainWindow.cpp
    main.cpp
)

add_executable(${target} ${sources})

#
target_include_directories(${target} PUBLIC ${GTKMM_INCLUDE_DIRS})
target_link_directories(${target} PUBLIC ${GTKMM_LIBRARY_DIRS})
target_link_libraries(${target} ${GTK4_LIBRARIES} ${GTKMM_LIBRARIES})
