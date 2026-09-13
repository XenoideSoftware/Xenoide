
#include "main_window_model.h"

#include <filesystem>
#include <numeric>
#include <stdexcept>


MainWindowModel::MainWindowModel(const SciEditor &editor, const std::optional<std::string> &filePath) : editor(editor), filePath(filePath) {
    languageConfigMap["c++"] = {
        SCLEX_CPP,
        "alignas alignof and and_eq asm atomic_cancel atomic_commit atomic_noexcept auto bitand bitor bool "
        "break case catch char char16_t char32_t class compl concept const constexpr const_cast continue "
        "decltype default delete do double dynamic_cast else enum explicit export extern false float for "
        "friend goto if inline int import long module mutable namespace new noexcept not not_eq nullptr "
        "operator or or_eq private protected public register reinterpret_cast requires return short signed "
        "sizeof static static_assert static_cast struct switch synchronized template this thread_local throw "
        "true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq", {
            {SCE_C_COMMENT, makeRGB(0, 128, 0)},
        {SCE_C_COMMENTLINE, makeRGB(0, 128, 0)},
        {SCE_C_WORD, makeRGB(0, 0, 255)},
        {SCE_C_STRING, makeRGB(163, 21, 21)},
        {SCE_C_NUMBER, makeRGB(128, 0, 128)},
    },
    "C/C++ Files",
        {"*.cpp", "*.c", "*.cc", "*.c++", "*.hpp", "*.h", "*.hh", "*.h++"}
    };

    // glsl 4.5 keywords
    languageConfigMap["glsl"] = {
        SCLEX_CPP,
        // common keywords
        "const uniform buffer shared attribute varying coherent volatile restrict readonly writeonly atomic_uint layout centroid flat smooth noperspective patch sample invariant "
        "precise break continue do for while switch case default if else subroutine in out inout int void bool true false float double discard return vec2 vec3 vec4 ivec2 ivec3 "
        "ivec4 bvec2 bvec3 bvec4 uint uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 mat2 mat3 mat4 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 dmat2 dmat3 dmat4 dmat2x2 "
        "dmat2x3 dmat2x4 dmat3x2 dmat3x3 dmat3x4 dmat4x2 dmat4x3 dmat4x4 lowp mediump highp precision sampler1D sampler1DShadow sampler1DArray sampler1DArrayShadow isampler1D "
        "isampler1DArray usampler1D usampler1DArray sampler2D sampler2DShadow sampler2DArray sampler2DArrayShadow isampler2D isampler2DArray usampler2D usampler2DArray "
        "sampler2DRect sampler2DRectShadow isampler2DRect usampler2DRect sampler2DMS isampler2DMS usampler2DMS sampler2DMSArray isampler2DMSArray usampler2DMSArray sampler3D "
        "isampler3D usampler3D samplerCube samplerCubeShadow isamplerCube usamplerCube samplerCubeArray samplerCubeArrayShadow isamplerCubeArray usamplerCubeArray samplerBuffer "
        "isamplerBuffer usamplerBuffer image1D iimage1D uimage1D image1DArray iimage1DArray uimage1DArray image2D iimage2D uimage2D image2DArray iimage2DArray uimage2DArray "
        "image2DRect iimage2DRect uimage2DRect image2DMS iimage2DMS uimage2DMS image2DMSArray iimage2DMSArray uimage2DMSArray image3D iimage3D uimage3D imageCube iimageCube "
        "uimageCube imageCubeArray iimageCubeArray uimageCubeArray imageBuffer iimageBuffer uimageBuffer struct "

        // vulkan keywords
        "texture1D texture1DArray itexture1D itexture1DArray utexture1D utexture1DArray texture2D texture2DArray itexture2D itexture2DArray utexture2D utexture2DArray "
        "texture2DRect itexture2DRect utexture2DRect texture2DMS itexture2DMS utexture2DMS texture2DMSArray itexture2DMSArray utexture2DMSArray texture3D itexture3D utexture3D "
        "textureCube itextureCube utextureCube textureCubeArray itextureCubeArray utextureCubeArray textureBuffer itextureBuffer utextureBuffer sampler samplerShadow subpassInput "
        "isubpassInput usubpassInput subpassInputMS isubpassInputMS usubpassInputMS "

        // reserved 
        "common partition active asm class union enum typedef template this resource goto inline noinline public static extern external interface long short half fixed unsigned "
        "superp input output hvec2 hvec3 hvec4 fvec2 fvec3 fvec4 filter sizeof cast namespace using sampler3DRect ", {
            {SCE_C_COMMENT, makeRGB(0, 128, 0)},
        {SCE_C_COMMENTLINE, makeRGB(0, 128, 0)},
        {SCE_C_WORD, makeRGB(0, 0, 255)},
        {SCE_C_STRING, makeRGB(163, 21, 21)},
        {SCE_C_NUMBER, makeRGB(128, 0, 128)},
    },
    "GLSL Shaders",
        {"*.glsl", "*.vert", "*.frag", "*.prog", "*.tess", "*.geom"}
    };

    languageConfigMap["cmake"] = {
        SCLEX_CMAKE,

        // keywords
        "add_custom_command add_custom_target add_definitions add_dependencies add_executable add_library " 
        "add_subdirectory add_test aux_source_directory build_command build_name cmake_minimum_required " 
        "configure_file create_test_sourcelist else elseif enable_language enable_testing endforeach endif " 
        "endmacro endwhile exec_program execute_process export_library_dependencies file find_file find_library " 
        "find_package find_path find_program fltk_wrap_ui foreach get_cmake_property get_directory_property " 
        "get_filename_component get_source_file_property get_target_property get_test_property if include include_directories " 
        "include_external_msproject include_regular_expression install install_files install_programs install_targets " 
        "link_directories link_libraries list load_cache load_command macro make_directory mark_as_advanced " 
        "math message option output_required_files project qt_wrap_cpp qt_wrap_ui remove remove_definitions " 
        "separate_arguments set set_directory_properties set_source_files_properties set_target_properties set_tests_properties " 
        "site_name source_group string subdir_depends subdirs target_link_libraries try_compile try_run " 
        "use_mangled_mesa utility_source variable_requires vtk_make_instantiator vtk_wrap_java vtk_wrap_python " 
        "vtk_wrap_tcl while write_file "

        // keywords2
        "ABSOLUTE ABSTRACT ADDITIONAL_MAKE_CLEAN_FILES ALL AND APPEND ARGS ASCII BEFORE CACHE CACHE_VARIABLES CLEAR COMMAND COMMANDS "
        "COMMAND_NAME COMMENT COMPARE COMPILE_FLAGS COPYONLY DEFINED DEFINE_SYMBOL DEPENDS DOC EQUAL ESCAPE_QUOTES EXCLUDE EXCLUDE_FROM_ALL "
        "EXISTS EXPORT_MACRO EXT EXTRA_INCLUDE FATAL_ERROR FILE FILES FORCE FUNCTION GENERATED GLOB GLOB_RECURSE GREATER GROUP_SIZE "
        "HEADER_FILE_ONLY HEADER_LOCATION IMMEDIATE INCLUDES INCLUDE_DIRECTORIES INCLUDE_INTERNALS INCLUDE_REGULAR_EXPRESSION LESS "
        "LINK_DIRECTORIES LINK_FLAGS LOCATION MACOSX_BUNDLE MACROS MAIN_DEPENDENCY MAKE_DIRECTORY MATCH MATCHALL MATCHES MODULE "
        "NAME NAME_WE NOT NOTEQUAL NO_SYSTEM_PATH OBJECT_DEPENDS OPTIONAL OR OUTPUT OUTPUT_VARIABLE PATH PATHS POST_BUILD POST_INSTALL_SCRIPT "
        "PREFIX PREORDER PRE_BUILD PRE_INSTALL_SCRIPT PRE_LINK PROGRAM PROGRAM_ARGS PROPERTIES QUIET RANGE READ REGEX REGULAR_EXPRESSION REPLACE "
        "REQUIRED RETURN_VALUE RUNTIME_DIRECTORY SEND_ERROR SHARED SOURCES STATIC STATUS STREQUAL STRGREATER STRLESS SUFFIX TARGET TOLOWER TOUPPER "
        "VAR VARIABLES VERSION WIN32 WRAP_EXCLUDE WRITE APPLE MINGW MSYS CYGWIN BORLAND WATCOM MSVC MSVC_IDE MSVC60 MSVC70 MSVC71 MSVC80 "
        "CMAKE_COMPILER_2005 OFF ON", {
            {SCE_CMAKE_DEFAULT, makeRGB(0, 0, 0)},
        {SCE_CMAKE_COMMENT, makeRGB(0, 128, 0)},
        {SCE_CMAKE_STRINGDQ, makeRGB(128, 50, 0)},
        {SCE_CMAKE_STRINGLQ, makeRGB(128, 0, 50)},
        {SCE_CMAKE_STRINGRQ, makeRGB(128, 0, 0)},
        {SCE_CMAKE_COMMANDS, makeRGB(0, 0, 128)},
        {SCE_CMAKE_PARAMETERS, makeRGB(0, 0, 0)},
        {SCE_CMAKE_VARIABLE, makeRGB(0, 0, 0)},
        {SCE_CMAKE_USERDEFINED, makeRGB(0, 0, 0)},
        {SCE_CMAKE_WHILEDEF, makeRGB(0, 0, 0)},
        {SCE_CMAKE_FOREACHDEF, makeRGB(0, 0, 0)},
        {SCE_CMAKE_IFDEFINEDEF, makeRGB(0, 0, 128)},
        {SCE_CMAKE_MACRODEF, makeRGB(0, 0, 0)},
        {SCE_CMAKE_STRINGVAR, makeRGB(0, 0, 0)},
        {SCE_CMAKE_NUMBER, makeRGB(0, 0, 0)}
    },
        "CMake Files",
        {"CMakeLists.txt", "*.cmake"}
    };
}


std::string MainWindowModel::getFileFilter() const {
    std::vector<std::string> filters;

    for (const auto &pair : languageConfigMap) {
        std::string const &caption = pair.second.filePatternCaption;
        std::vector<std::string> const &patterns = pair.second.filePatterns;

        std::string const joinedPatternsCaption = join(patterns, std::string{","});
        std::string const joinedPatterns = join(patterns, std::string{";"});
        std::string const filter = caption + " (" + joinedPatternsCaption + ")|" + joinedPatterns;
        filters.push_back(filter);
    }

    filters.emplace_back("Text Files (*.txt)|*.txt");
    filters.emplace_back("All Files (*.*)|*.*");

    return join(filters, std::string{"|"});
}

std::map<std::string, LanguageConfig>::const_iterator MainWindowModel::detectLanguage(const std::string &path) const {
    std::string const fileName = std::filesystem::path(path).filename().string();

    return std::find_if(languageConfigMap.begin(), languageConfigMap.end(), [fileName](const auto pair) {
        for (std::string const pattern : pair.second.filePatterns) {
            if (wildcardMatch(pattern.c_str(), fileName.c_str())) {
                return true;
            }
        }

        return false;
        });
}

std::string MainWindowModel::getEditorTitle() const {
    namespace fs = std::filesystem;

    std::string const fileName = fs::path((filePath.value_or("Untitled"))).filename().string();

	return fileName + (isModified() ? "*" : "");
}

std::optional<std::string> MainWindowModel::getFilePath() const {
	return filePath;
}

bool MainWindowModel::canSave() const {
	return filePath.has_value();
}

bool MainWindowModel::isModified() const {
	return editor.getModify();
}

void MainWindowModel::new_() {
	filePath.reset();

	editor.clearAll();
	editor.clearState();
    updateLexer();

	MainWindowNotification notification;
	notification.filePathChanged = true;
	notification.modifiedFlagChanged = true;
	notification.undoBufferChanged = true;

	notify(notification);
}

void MainWindowModel::save(const std::optional<std::string> &newFilePath) {
	MainWindowNotification notification;
	notification.filePathChanged = filePath != newFilePath;
	notification.modifiedFlagChanged = true;

	filePath = newFilePath;

	if (!filePath) {
		throw std::runtime_error("");
	}

	xenoide::FileService fileService;
	fileService.save(*filePath, editor.getText());

	editor.setSavePoint();
    
    updateLexer();

	notify(notification);
}

void MainWindowModel::load(const std::optional<std::string> &newFilePath) {
	MainWindowNotification notification;
	notification.filePathChanged = filePath != newFilePath;
	notification.modifiedFlagChanged = true;
	notification.undoBufferChanged = true;

	filePath = newFilePath;

	if (!filePath) {
		throw std::runtime_error("");
	}

	xenoide::FileService fileService;

	editor.setText(fileService.load(*filePath));
	editor.setSavePoint();
	editor.clearState();

    updateLexer();

	notify(notification);
}


void MainWindowModel::updateLexer() {
    if (!filePath) {
        editor.syncLexerConfig({});
        return;
    }

    auto const languageConfigIt = detectLanguage(*filePath);
    if (languageConfigIt == languageConfigMap.end()) {
        editor.syncLexerConfig({});
        return;
    }

    auto &config = languageConfigIt->second;
    editor.syncLexerConfig(SciLexerConfig{config.lexer, config.keywords, config.styles});
}
