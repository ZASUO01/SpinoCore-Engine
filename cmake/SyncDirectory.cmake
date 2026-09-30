function(target_sync_directory TARGET_NAME SOURCE_DIR DEST_DIR)
    if(NOT EXISTS "${SOURCE_DIR}")
        file(MAKE_DIRECTORY "${SOURCE_DIR}")
    endif()

    set(SYNC_SCRIPT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_AssetSync.cmake")

    file(WRITE "${SYNC_SCRIPT_PATH}" "
        file(MAKE_DIRECTORY \"${DEST_DIR}\")
        file(GLOB_RECURSE ASSETS \"${SOURCE_DIR}/*\")

        foreach(SRC_FILE IN LISTS ASSETS)
            file(RELATIVE_PATH REL_PATH \"${SOURCE_DIR}\" \"\${SRC_FILE}\")
            set(DEST_FILE \"${DEST_DIR}/\${REL_PATH}\")

            get_filename_component(DEST_SUBDIR \"\${DEST_FILE}\" DIRECTORY)
            file(MAKE_DIRECTORY \"\${DEST_SUBDIR}\")

            execute_process(
                COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${SRC_FILE}\" \"\${DEST_FILE}\"
            )
        endforeach()
    ")

    set(SYNC_TARGET_NAME "${TARGET_NAME}_SyncAssets")
    add_custom_target(${SYNC_TARGET_NAME}
            COMMAND ${CMAKE_COMMAND} -P "${SYNC_SCRIPT_PATH}"
            COMMENT ""
            VERBATIM
    )

    add_dependencies(${TARGET_NAME} ${SYNC_TARGET_NAME})

endfunction()