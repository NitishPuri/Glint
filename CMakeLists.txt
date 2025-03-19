project(Glint)
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################## Dependencies ##################

# Vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

# GLFW
set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
set(GLFW_INCLUDE ${GLFW_PATH}/include)
set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)

################## Dependencies ##################

file(GLOB SOURCES
    src/*.cpp
    src/shaders/*.vert
    src/shaders/*.frag
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${GLFW_INCLUDE}
)


target_link_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIB_DIR})
target_link_libraries(${PROJECT_NAME}
    glfw3
    ${Vulkan_LIBRARIES}
)
