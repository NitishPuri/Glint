add_executable(glint_samples
    main.cpp
    sample.cpp
    sample_manager.cpp
    cube_sample.cpp
    textured_quad.cpp
    rotating_sample.cpp
)





target_link_libraries(glint_samples PRIVATE
    glint_core
    glint_ui
)

target_compile_features(glint_samples PRIVATE cxx_std_20)
