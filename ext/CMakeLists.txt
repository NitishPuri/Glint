cmake_minimum_required(VERSION 3.25)


# GLFW - Pre-compiled binaries
add_library(glfw INTERFACE)
set(GLFW_PATH ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4.bin.WIN64)
target_include_directories(glfw INTERFACE ${GLFW_PATH}/include)
target_link_directories(glfw INTERFACE ${GLFW_PATH}/lib-vc2022)
target_link_libraries(glfw INTERFACE glfw3)

# STB - headers only
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# TinyObjLoader - headers only
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)

# arcball camera - headers only
add_library(arcball_camera INTERFACE)
target_include_directories(arcball_camera INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/arcball_camera)


# Imgui - compiled as a static library
add_library(imgui STATIC
    imgui/imgui.cpp
    # imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Find and link required libraries for ImGui
# find_package(Vulkan REQUIRED)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

# ImGui needs GLFW
# if(NOT TARGET glfw)
#     # Try to find existing GLFW or add your GLFW subdirectory here
#     find_package(glfw3 REQUIRED)
#     target_link_libraries(imgui PUBLIC glfw)
# endif()