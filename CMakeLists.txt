cmake_minimum_required(VERSION 3.10)

set(NAME Glint)


set(CMAKE_CXX_STANDARD 20)
project(${NAME} VERSION 0.0.1)

################## Dependencies ##################
# GLFW
set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
set(GLFW_INCLUDE ${GLFW_PATH}/include)
set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)

# ImGui
set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/ext/imgui)
file(GLOB_RECURSE IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)


# GLAD
set(GLAD_PATH ${PROJECT_SOURCE_DIR}/ext/glad)
set(GLAD_INCLUDE ${GLAD_PATH}/include)
file(GLOB_RECURSE GLAD_SOURCES ${GLAD_PATH}/src/*.c)

# GLM
set(GLM_PATH ${PROJECT_SOURCE_DIR}/ext/glm)
set(GLM_INCLUDE ${GLM_PATH})
################## Dependencies ##################



################## Core ##################


# Core
# file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Source files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_sources(${PROJECT_NAME} PRIVATE ${GLAD_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})


add_compile_definitions(${PROJECT_NAME} PUBLIC ROOT="${PROJECT_SOURCE_DIR}")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_link_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
glfw3
opengl32.lib
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/ext
    ${GLAD_INCLUDE}
    ${GLFW_INCLUDE}
    ${GLM_INCLUDE}
)

# Enable warnings
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

################## Core ##################