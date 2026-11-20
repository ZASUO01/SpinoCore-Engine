spinocore_add_test(SpinoCore-E2ETests LABEL E2E)

if(TARGET SpinoCore-E2ETests)
    target_link_libraries(SpinoCore-E2ETests PRIVATE SDL3::SDL3-static)
endif()
