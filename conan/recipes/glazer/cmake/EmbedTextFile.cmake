# Helper for embedding text files (inja templates, static asset headers, etc.)
# into C++ static libraries as raw string literals. Uses a CMake script mode
# invocation via add_custom_command so edits to the input file trigger a
# rebuild of the generated .cpp.
#
# Usage:
#     include("${CMAKE_SOURCE_DIR}/cmake/EmbedTextFile.cmake")
#     glaze_embed_text_file(
#         "path/to/input.inja"
#         "${CMAKE_CURRENT_BINARY_DIR}/generated/gl_hpp_inja.cpp"
#         "glaze::cgen::detail::templates"
#         "gl_hpp_inja"
#     )
#
# The generated .cpp defines a single const char * with the template contents.
# Callers must also add the generated .cpp to their add_library() sources.
function(glaze_embed_text_file input_file output_cpp cpp_namespace symbol_name)
    add_custom_command(
        OUTPUT  "${output_cpp}"
        COMMAND "${CMAKE_COMMAND}"
                -DINPUT_FILE=${input_file}
                -DOUTPUT_CPP=${output_cpp}
                -DCPP_NAMESPACE=${cpp_namespace}
                -DSYMBOL_NAME=${symbol_name}
                -P "${CMAKE_SOURCE_DIR}/cmake/EmbedTextFileScript.cmake"
        DEPENDS "${input_file}"
                "${CMAKE_SOURCE_DIR}/cmake/EmbedTextFileScript.cmake"
        COMMENT "Embedding ${input_file} as ${symbol_name}"
        VERBATIM
    )
endfunction()
