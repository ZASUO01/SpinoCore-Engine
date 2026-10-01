function(target_sync_directory TARGET_NAME SOURCE_DIR DEST_DIR)
    if(NOT EXISTS "${SOURCE_DIR}")
        file(MAKE_DIRECTORY "${SOURCE_DIR}")
    endif()

    string(MD5 PATH_HASH "${SOURCE_DIR}_${DEST_DIR}")
    string(SUBSTRING "${PATH_HASH}" 0 8 SHORT_HASH)

    set(SYNC_TARGET_NAME "${TARGET_NAME}_SyncAssets_${SHORT_HASH}")
    set(SYNC_SCRIPT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${SYNC_TARGET_NAME}.cmake")


    file(WRITE "${SYNC_SCRIPT_PATH}" "
        file(COPY \"${SOURCE_DIR}/\" DESTINATION \"${DEST_DIR}\")
    ")

    if(NOT TARGET ${SYNC_TARGET_NAME})
        add_custom_target(${SYNC_TARGET_NAME}
                COMMAND ${CMAKE_COMMAND} -P "${SYNC_SCRIPT_PATH}"
                COMMENT "Syncing ${SOURCE_DIR} to ${DEST_DIR}"
                VERBATIM
        )

        add_dependencies(${TARGET_NAME} ${SYNC_TARGET_NAME})
    endif()
endfunction()