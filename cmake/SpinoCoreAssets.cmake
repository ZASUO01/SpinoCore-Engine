# SPDX-License-Identifier: MIT
# Copyright (c) 2026 ZASUO01

function(spinocore_add_assets target)
    set(SPINOCORE_ASSETS_DIRECTORY_NAME "Assets")
    set(SPINOCORE_SYNC_ASSETS_SCRIPT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/SyncAssets.cmake")

    if(NOT TARGET ${target})
        message(FATAL_ERROR "spinocore_add_assets: \"${target}\" is not a target.")
    endif()

    set(source "${CMAKE_SOURCE_DIR}/${SPINOCORE_ASSETS_DIRECTORY_NAME}")
    if(NOT IS_DIRECTORY "${source}")
        file(MAKE_DIRECTORY "${source}")
        message(STATUS "SpinoCore: created the assets folder ${source}")
    endif()

    add_custom_target(${target}-SyncAssets
            COMMAND "${CMAKE_COMMAND}"
                "-DSOURCE_DIR=${source}"
                "-DDESTINATION_DIR=$<TARGET_FILE_DIR:${target}>/${SPINOCORE_ASSETS_DIRECTORY_NAME}"
                -P "${SPINOCORE_SYNC_ASSETS_SCRIPT}"
            COMMENT "Syncing ${SPINOCORE_ASSETS_DIRECTORY_NAME} for ${target}"
            VERBATIM
    )
    set_target_properties(${target}-SyncAssets PROPERTIES FOLDER "SpinoCore")
    add_dependencies(${target} ${target}-SyncAssets)
endfunction()
