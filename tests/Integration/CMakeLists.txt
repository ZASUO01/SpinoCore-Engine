spinocore_add_test(SpinoCore-IntegrationTests LABEL Integration)

# Integration tests talk to SDL directly to observe the engine's effects on it
if(TARGET SpinoCore-IntegrationTests)
    target_link_libraries(SpinoCore-IntegrationTests PRIVATE SDL3::SDL3-static)
endif()
