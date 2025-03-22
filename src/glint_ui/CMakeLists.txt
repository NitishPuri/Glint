add_library(glint_ui STATIC
    imgui_manager.cpp
)

target_include_directories(glint_ui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with required libraries
target_link_libraries(glint_ui PUBLIC
    glint_core
    imgui
)

target_compile_features(glint_ui PRIVATE cxx_std_20)
