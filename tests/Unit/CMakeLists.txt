spinocore_add_test(SpinoCore-UnitTests LABEL Unit)

if(TARGET SpinoCore-UnitTests)
    target_link_libraries(SpinoCore-UnitTests PRIVATE SDL3::SDL3-static)
endif()
