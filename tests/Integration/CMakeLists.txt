set(INTEGRATION_SOURCES
#FileSystem
    "FileSystem/FileHelpers_test.cpp"
)

add_executable("SpinoCoreIntegrationTests" ${INTEGRATION_SOURCES})

target_include_directories("SpinoCoreIntegrationTests"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
)

target_link_libraries("SpinoCoreIntegrationTests"
        PRIVATE
        SpinoCore::Runtime
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests("SpinoCoreIntegrationTests" PROPERTIES LABELS "Integration")