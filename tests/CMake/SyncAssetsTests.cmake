# SPDX-License-Identifier: MIT
# Copyright (c) 2026 ZASUO01
#
# Tests for cmake/SyncAssets.cmake. Run as: cmake -DSYNC_SCRIPT=<path> -DWORK_DIR=<folder> -P SyncAssetsTests.cmake
cmake_minimum_required(VERSION 3.24)

set(src "${WORK_DIR}/Project/Assets")
set(dst "${WORK_DIR}/bin/Assets")
file(REMOVE_RECURSE "${WORK_DIR}")

set(failures 0)
macro(fail message)
    message(SEND_ERROR "FAIL: ${message}")
    math(EXPR failures "${failures} + 1")
endmacro()

# Runs the sync and stores its output in syncOutput.
function(run_sync)
    execute_process(
            COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${src}" "-DDESTINATION_DIR=${dst}" -P "${SYNC_SCRIPT}"
            RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "sync failed: ${err}")
    endif()
    set(syncOutput "${out}${err}" PARENT_SCOPE)
endfunction()

function(write path content)
    get_filename_component(dir "${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${dir}")
    file(WRITE "${path}" "${content}")
endfunction()

# Every file and folder under a root, relative and sorted, with file contents, so two trees compare as strings.
function(snapshot root outVar)
    file(GLOB_RECURSE entries LIST_DIRECTORIES true RELATIVE "${root}" "${root}/*")
    list(SORT entries)
    set(result "")
    foreach(entry IN LISTS entries)
        if(IS_DIRECTORY "${root}/${entry}")
            string(APPEND result "D:${entry}\n")
        else()
            file(READ "${root}/${entry}" content)
            string(APPEND result "F:${entry}=${content}\n")
        endif()
    endforeach()
    set(${outVar} "${result}" PARENT_SCOPE)
endfunction()

macro(expect_mirror label)
    snapshot("${src}" srcTree)
    snapshot("${dst}" dstTree)
    if(NOT srcTree STREQUAL dstTree)
        fail("${label}: destination is not a mirror.\n--- source ---\n${srcTree}--- destination ---\n${dstTree}")
    endif()
endmacro()

# 1. A missing source folder is created, and the destination starts empty.
run_sync()
if(NOT IS_DIRECTORY "${src}")
    fail("missing source folder was not created")
endif()
if(NOT IS_DIRECTORY "${dst}")
    fail("destination folder was not created")
endif()
expect_mirror("empty source")

# 2. Files at any depth are copied, including hidden files and names with spaces.
write("${src}/textures/player.png" "player-v1")
write("${src}/textures/ui/buttons/ok button.png" "ok")
write("${src}/texts/intro.txt" "Hello")
write("${src}/.hidden" "secret")
file(MAKE_DIRECTORY "${src}/empty/nested")
run_sync()
expect_mirror("initial copy")

# 3. Running again with no changes copies and removes nothing.
run_sync()
if(syncOutput MATCHES "copied")
    fail("unchanged source should not copy anything: ${syncOutput}")
endif()

# 4. Changed contents are copied again.
write("${src}/textures/player.png" "player-v2")
run_sync()
expect_mirror("changed file")
if(NOT syncOutput MATCHES "1 copied, 0 removed")
    fail("only the changed file should be copied: ${syncOutput}")
endif()

# 5. Deleted files and folders, at any depth, are deleted from the copy.
file(REMOVE "${src}/texts/intro.txt")
file(REMOVE_RECURSE "${src}/textures/ui")
file(REMOVE_RECURSE "${src}/empty/nested")
run_sync()
expect_mirror("deletions")
if(EXISTS "${dst}/textures/ui")
    fail("removed folder still in the copy")
endif()

# 6. A path that changes between file and folder is replaced in the copy.
file(REMOVE "${src}/textures/player.png")
write("${src}/textures/player.png/frame0.png" "frame0")
file(REMOVE_RECURSE "${src}/empty")
write("${src}/empty" "now a file")
run_sync()
expect_mirror("file/folder swaps")

# 7. Files placed only in the copy are removed.
write("${dst}/stray.txt" "stray")
write("${dst}/stray-folder/inner.txt" "stray")
run_sync()
expect_mirror("strays in the copy")

# 8. Deleting the whole source folder recreates it empty and empties the copy.
file(REMOVE_RECURSE "${src}")
run_sync()
if(NOT IS_DIRECTORY "${src}")
    fail("deleted source folder was not recreated")
endif()
expect_mirror("deleted source folder")

# 9. Missing arguments fail loudly.
execute_process(COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${src}" -P "${SYNC_SCRIPT}"
        RESULT_VARIABLE missingArg OUTPUT_QUIET ERROR_QUIET)
if(missingArg EQUAL 0)
    fail("missing DESTINATION_DIR should fail")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
if(failures GREATER 0)
    message(FATAL_ERROR "${failures} SyncAssets check(s) failed")
endif()
message(STATUS "SyncAssets: all checks passed")
