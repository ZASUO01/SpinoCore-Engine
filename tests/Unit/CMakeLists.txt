set(UNIT_SOURCES
#Config
    "Config/ConfigKey_test.cpp"
    "Config/Store_test.cpp"
#FileSystem
    "FileSystem/VirtualFileSystem_test.cpp"
    "FileSystem/VirtualPath_test.cpp"
#Logs
    "Logs/Logs_test.cpp"
#Utils
    "Utils/ConstString_test.cpp"
    "Utils/JSONUtils_test.cpp"
    "Utils/ThreadSafeQueue_test.cpp"
)

add_executable("SpinoCoreUnitTests" ${UNIT_SOURCES})

target_include_directories("SpinoCoreUnitTests"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor"
)

target_link_libraries("SpinoCoreUnitTests"
        PRIVATE
        SpinoCore::Runtime
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests("SpinoCoreUnitTests" PROPERTIES LABELS "Unit")