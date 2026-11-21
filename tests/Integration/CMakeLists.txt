spinocore_add_test(SpinoCore-IntegrationTests LABEL Integration)

if(TARGET SpinoCore-IntegrationTests)
    target_link_libraries(SpinoCore-IntegrationTests PRIVATE SDL3::SDL3-static SDL3_image::SDL3_image-static SDL3_ttf::SDL3_ttf-static SpinoCore::glad)
endif()
