project(Glint)
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################## Dependencies ##################

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
set(GLFW_INCLUDE ${GLFW_PATH}/include)
set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)

# STB
set(STB_INCLUDE ${PROJECT_SOURCE_DIR}/ext/stb)

################## Dependencies ##################

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
)
    
file (GLOB_RECURSE SHADERS
    src/shaders/*.vert
    src/shaders/*.frag
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${GLFW_INCLUDE}
    ${Vulkan_INCLUDE_DIRS}
    ${STB_INCLUDE}
    ${PROJECT_SOURCE_DIR}/src
)


target_link_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIB_DIR})
target_link_libraries(${PROJECT_NAME}
    glfw3
    ${Vulkan_LIBRARIES}
)
