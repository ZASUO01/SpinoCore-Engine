add_library(glad STATIC
        "src/glad.c"
        "include/glad/glad.h"
        "include/KHR/khrplatform.h"
)
add_library(SpinoCore::glad ALIAS glad)

target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(glad PROPERTIES
        C_STANDARD 99
        POSITION_INDEPENDENT_CODE ON
        FOLDER "SpinoCore/ThirdParty"
)