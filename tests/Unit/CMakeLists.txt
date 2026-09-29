set(UNIT_SOURCES
#Logs
    "Logs/Logs_test.cpp"
)

add_executable("SpinoCoreUnitTests" ${UNIT_SOURCES})

target_include_directories("SpinoCoreUnitTests"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
)

target_link_libraries("SpinoCoreUnitTests"
        PRIVATE
        SpinoCore::Runtime
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests("SpinoCoreUnitTests" PROPERTIES LABELS "Unit")