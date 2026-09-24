# Provides winlamb::manifest, an INTERFACE IMPORTED target carrying the
# bundled Win32 application manifest as an INTERFACE source. Linking a
# WIN32 executable against it makes MSVC embed the manifest in the .exe
# (Common Controls 6, asInvoker, DPI-aware).

get_filename_component(_winlamb_manifest_file
    "${CMAKE_CURRENT_LIST_DIR}/../../../res/win10.exe.manifest" ABSOLUTE)

if(NOT TARGET winlamb::manifest)
    add_library(winlamb::manifest INTERFACE IMPORTED)
    set_property(TARGET winlamb::manifest APPEND PROPERTY
        INTERFACE_SOURCES "${_winlamb_manifest_file}")
endif()

unset(_winlamb_manifest_file)
