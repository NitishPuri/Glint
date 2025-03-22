cmake_minimum_required(VERSION 3.25)

add_library(glint_core STATIC
    core/window.cpp
    core/utils.cpp
    core/config.cpp
    renderer/command_manager.cpp
    renderer/vertex.cpp
    renderer/mesh.cpp
    renderer/mesh_factory.cpp
    renderer/pipeline.cpp
    renderer/render_pass.cpp
    renderer/renderer.cpp
    renderer/swapchain.cpp
    renderer/synchronization_manager.cpp
    renderer/vk_context.cpp
    renderer/vk_utils.cpp
    renderer/descriptor.cpp    
    renderer/texture.cpp
)

target_include_directories(glint_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(glint_core PRIVATE cxx_std_20)


# Find and link Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(glint_core PUBLIC 
    Vulkan::Vulkan 
    glfw 
    stb 
    tinyobjloader
)

# TODO: Use path shader bin path relative to bin path 
// and move the bins to a single folder eassier to package
target_compile_definitions(glint_core PRIVATE 
    SHADER_BIN_DIR="${CMAKE_BINARY_DIR}/bin/shaders/"
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
)
