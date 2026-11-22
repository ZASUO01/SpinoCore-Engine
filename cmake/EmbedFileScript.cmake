file(READ "${INPUT}" hex HEX)
string(LENGTH "${hex}" hexLength)
math(EXPR size "${hexLength} / 2")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
string(REGEX REPLACE "((0x[0-9a-f][0-9a-f],){32})" "\\1\n        " bytes "${bytes}")
file(WRITE "${OUTPUT}" "// Generated from ${INPUT}. Do not edit.
#include <cstddef>

namespace ${NAMESPACE} {
    extern const unsigned char ${NAME}[];
    extern const std::size_t ${NAME}Size;

    const unsigned char ${NAME}[] = {
        ${bytes}
    };
    const std::size_t ${NAME}Size = ${size};
}
")
