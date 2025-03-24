
file (GLOB_RECURSE SHADERS
    base.vert
    base.frag
    baseMVP.vert
    basic_tex.vert
    basic_tex.frag
    dynamic_uniform_buffer.vert
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