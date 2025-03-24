
set(GLINT_SAMPLE_SOURCES 
    sample_main.cpp
    sample.h
    sample.cpp
    sample_manager.h
    sample_manager.cpp
    textured_quad.h
    textured_quad.cpp
    rotating_sample.h
    rotating_sample.cpp
    cube_sample.h
    cube_sample.cpp
    dynamic_uniform_buffer.h
    dynamic_uniform_buffer.cpp
)

add_executable(glint_samples
    ${GLINT_SAMPLE_SOURCES}
)


target_link_libraries(glint_samples PRIVATE
    glint_core
    glint_ui
)

target_compile_features(glint_samples PRIVATE cxx_std_20)

set_target_properties(glint_samples PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_compile_definitions(glint_samples PRIVATE 
    GLM_FORCE_RADIANS 
    GLM_FORCE_DEPTH_ZERO_TO_ONE 
    GLM_ENABLE_EXPERIMENTAL
)

