# Script-mode helper invoked by glaze_embed_text_file via add_custom_command.
# Reads INPUT_FILE and writes OUTPUT_CPP containing a single C++ raw string
# literal named SYMBOL_NAME inside namespace CPP_NAMESPACE.
#
# All four arguments are passed via -D on the cmake command line, so they are
# available as plain CMake variables here.

file(READ "${INPUT_FILE}" file_content)

# Unique delimiter that must not appear in any Glaze template.
set(delimiter "GLAZE_EMBED")

set(header
"// Auto-generated from ${INPUT_FILE} by glaze_embed_text_file().
// Any edits here will be overwritten on the next build.
namespace ${CPP_NAMESPACE} {
extern const char *${SYMBOL_NAME};
const char *${SYMBOL_NAME} = R\"${delimiter}(")
set(footer
")${delimiter}\";
} // namespace
")

file(WRITE "${OUTPUT_CPP}" "${header}${file_content}${footer}")
