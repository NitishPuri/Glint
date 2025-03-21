cmake_minimum_required(VERSION 3.25)
project(Glint)

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


################## Shader Compilation ##################
file (GLOB_RECURSE SHADERS
    src/shaders/*.vert
    src/shaders/*.frag
)

# Create shader output directory
set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/shaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})



# Custom shader compilation command
find_program(GLSLC glslc HINTS ${Vulkan_GLSLC_EXECUTABLE} 
                            $ENV{VULKAN_SDK}/Bin
                            ${Vulkan_INCLUDE_DIRS}/../Bin)

if(NOT GLSLC)
    message(FATAL_ERROR "Could not find glslc shader compiler. Please make sure Vulkan SDK is installed correctly.")
endif()

# Function to compile shaders
foreach(SHADER ${SHADERS})
    get_filename_component(FILENAME ${SHADER} NAME)
    set(SPIRV_OUTPUT "${SHADER_OUTPUT_DIR}/${FILENAME}.spv")
    
    # Add custom command to compile shaders
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${GLSLC} -o ${SPIRV_OUTPUT} ${SHADER}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${FILENAME}"
        VERBATIM
    )
    
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})
endforeach()


# Create a custom target for shader compilation
source_group("Shader Files" FILES ${SHADERS})
add_custom_target(
    shaders ALL
    DEPENDS ${SPIRV_BINARY_FILES}
    COMMENT "Compiling all shaders"
    SOURCES ${SHADERS}
)

# Make shader source files show up in Visual Studio
# add_custom_target(
#     shader_files SOURCES ${SHADERS}
# )
# add_dependencies(shaders shader_files)


# Copy shaders to output directory if needed
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${SHADER_OUTPUT_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
#    COMMENT "Copying compiled shaders to output directory"
#)

################## Shader Compilation ##################


################## Source Files ##################

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
)
    

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

add_executable(${PROJECT_NAME} ${SOURCES})

# Add dependency on shader compilation
add_dependencies(${PROJECT_NAME} shaders)   
# add_compile_definitions(SHADER_OUTPUT_DIR="${SHADER_OUTPUT_DIR}")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SHADER_BIN_DIR="${SHADER_OUTPUT_DIR}"
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
)



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
