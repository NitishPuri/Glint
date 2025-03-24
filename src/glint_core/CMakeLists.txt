cmake_minimum_required(VERSION 3.25)

set(GLINT_SOURCES 
    core/window.cpp
    core/config.cpp
    core/camera.cpp
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
    renderer/vk_tools.cpp
    renderer/descriptor.cpp    
    renderer/texture.cpp
)

set (GLINT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(GLINT_HEADERS
    core/window.h
    core/config.h
    core/camera.h
    renderer/command_manager.h
    renderer/vertex.h
    renderer/mesh.h
    renderer/mesh_factory.h
    renderer/pipeline.h
    renderer/render_pass.h
    renderer/renderer.h
    renderer/swapchain.h
    renderer/synchronization_manager.h
    renderer/vk_context.h
    renderer/vk_utils.h
    renderer/vk_tools.h
    renderer/descriptor.h    
    renderer/texture.h
)

add_library(glint_core STATIC
    ${GLINT_SOURCES}
    ${GLINT_HEADERS}
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
    arcball_camera
)

# TODO: Use path shader bin path relative to bin path 
# and move the bins to a single folder eassier to package
target_compile_definitions(glint_core PRIVATE
    BASE_DIR="${CMAKE_SOURCE_DIR}" 
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
)
