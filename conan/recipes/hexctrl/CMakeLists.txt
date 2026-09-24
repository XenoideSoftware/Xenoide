cmake_minimum_required(VERSION 3.20)
project(HexCtrlPackage CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(HEXCTRL_ROOT ${CMAKE_SOURCE_DIR}/HexCtrl)

add_library(HexCtrl STATIC
    ${HEXCTRL_ROOT}/src/res/HexCtrl.rc
    ${HEXCTRL_ROOT}/src/dep/HexCtrl_StrToNum.ixx
    ${HEXCTRL_ROOT}/src/dep/HexCtrl_ListEx.ixx
    ${HEXCTRL_ROOT}/src/HexUtility.ixx
    ${HEXCTRL_ROOT}/src/CHexScroll.ixx
    ${HEXCTRL_ROOT}/src/CHexSelection.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgBkmMgr.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgCodepage.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgDataInterp.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgGoTo.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgModify.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgProgress.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgSearch.ixx
    ${HEXCTRL_ROOT}/src/CHexDlgTemplMgr.ixx
    ${HEXCTRL_ROOT}/src/CHexCtrl.ixx
)

set_target_properties(HexCtrl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(HexCtrl PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /W3 /experimental:module>
)

target_include_directories(HexCtrl PUBLIC ${HEXCTRL_ROOT})
target_link_libraries(HexCtrl PUBLIC user32 comctl32 gdi32)
