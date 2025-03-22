cmake_minimum_required(VERSION 3.25)

add_library(glint_core STATIC
    core/window.cpp
    # core/logger.cpp
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


# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${GLFW_INCLUDE}
#     ${Vulkan_INCLUDE_DIRS}
#     ${STB_INCLUDE}
#     ${TINY_OBJ_LOADER_INCLUDE}
#     ${PROJECT_SOURCE_DIR}/src
# )


# Find and link Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(glint_core PUBLIC 
    Vulkan::Vulkan 
    glfw 
    stb 
    tinyobjloader
)

# Link with GLFW
# if(NOT TARGET glfw)
#     find_package(glfw3 REQUIRED)
#     target_link_libraries(glint_core PUBLIC glfw)
# endif()

# Add compile definitions if needed
# target_compile_definitions(glint_core PRIVATE
#     GLFW_INCLUDE_VULKAN
# )

target_compile_definitions(glint_core PRIVATE 
    # SHADER_BIN_DIR="${SHADER_OUTPUT_DIR}/"
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
)
