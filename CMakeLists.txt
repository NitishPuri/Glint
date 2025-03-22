cmake_minimum_required(VERSION 3.25)
project(Glint)

# Remove this, use targets instead
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")


# Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Please make sure Vulkan SDK is installed correctly.")
endif()

################## Dependencies ##################


# Add external dependencies
add_subdirectory(ext)


# # GLFW
# set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
# set(GLFW_INCLUDE ${GLFW_PATH}/include)
# set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)

# # STB
# set(STB_INCLUDE ${PROJECT_SOURCE_DIR}/ext/stb)

# # TinyObjLoader
# set(TINY_OBJ_LOADER_INCLUDE ${PROJECT_SOURCE_DIR}/ext/tinyobjloader)

# ImGui
# Add ImGui source files
# TODO: Move into a seperate library
# set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui)
# set(IMGUI_SOURCES
#     ${IMGUI_DIR}/imgui.cpp
#     # ${IMGUI_DIR}/imgui_demo.cpp
#     ${IMGUI_DIR}/imgui_draw.cpp
#     ${IMGUI_DIR}/imgui_tables.cpp
#     ${IMGUI_DIR}/imgui_widgets.cpp
#     ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
#     ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
# )

# Add ImGui include directories
# TODO: dont use include_directories, use targets instead
# include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

################## Dependencies ##################


################## Shader Compilation ##################
# TODO: Move this to a separate file in shaders directory
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

# Add Glint libraries
add_subdirectory(src/glint_core)


add_subdirectory(src/glint_ui)
# add_subdirectory(src/glint_renderer)

# add_subdirectory(src/shaders)

add_subdirectory(src/samples)
# 
# add_subdirectory(src/minimal)


################## Source Files ##################

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
    ${IMGUI_SOURCES}
)

# TODO: Remove this
# set(SOURCES
#     ${SOURCES}
#     src/ui/imgui_manager.cpp
#     ${IMGUI_SOURCES}
# )
    

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

# add_executable(${PROJECT_NAME} ${SOURCES})

# Add dependency on shader compilation
# add_dependencies(${PROJECT_NAME} shaders)   
# add_compile_definitions(SHADER_OUTPUT_DIR="${SHADER_OUTPUT_DIR}")

# target_compile_definitions(${PROJECT_NAME} PRIVATE 
#     SHADER_BIN_DIR="${SHADER_OUTPUT_DIR}/"
#     GLM_FORCE_DEPTH_ZERO_TO_ONE
#     GLM_FORCE_RADIANS
#     # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
# )



# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${GLFW_INCLUDE}
#     ${Vulkan_INCLUDE_DIRS}
#     ${STB_INCLUDE}
#     ${TINY_OBJ_LOADER_INCLUDE}
#     ${PROJECT_SOURCE_DIR}/src
# )

# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)


# target_link_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIB_DIR})
# target_link_libraries(${PROJECT_NAME}
#     glfw3
#     ${Vulkan_LIBRARIES}
# )
