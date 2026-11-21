# Compiles a file's bytes into a target as
#   namespace <NAMESPACE> { extern const unsigned char <NAME>[]; extern const std::size_t <NAME>Size; }
# The source is regenerated whenever the input file changes.
#
#   spinocore_embed_file(<target> INPUT <file> NAMESPACE <ns> NAME <symbol>)
set(SPINOCORE_EMBED_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/EmbedFileScript.cmake")

function(spinocore_embed_file target)
    cmake_parse_arguments(ARG "" "INPUT;NAMESPACE;NAME" "" ${ARGN})
    set(output "${CMAKE_CURRENT_BINARY_DIR}/embedded/${ARG_NAME}.cpp")
    add_custom_command(
            OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}"
                "-DINPUT=${ARG_INPUT}" "-DOUTPUT=${output}"
                "-DNAMESPACE=${ARG_NAMESPACE}" "-DNAME=${ARG_NAME}"
                -P "${SPINOCORE_EMBED_SCRIPT}"
            DEPENDS "${ARG_INPUT}" "${SPINOCORE_EMBED_SCRIPT}"
            COMMENT "Embedding ${ARG_INPUT}"
            VERBATIM)
    target_sources(${target} PRIVATE "${output}")
endfunction()
