cmake_minimum_required(VERSION 3.25)
project(Glint)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

# if(MSVC)
#   # Force consistent runtime library for all targets
#   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
  
#   # Add this to propagate the setting to all targets
#   add_compile_options(
#     $<$<CONFIG:>:/MD>
#     $<$<CONFIG:Debug>:/MDd>
#     $<$<CONFIG:Release>:/MD>
#   )
# endif()


# Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Please make sure Vulkan SDK is installed correctly.")
endif()

################## Dependencies ##################
# Add external dependencies
add_subdirectory(ext)
################## Dependencies ##################

################## Shader Compilation ##################
add_subdirectory(src/shaders)
################## Shader Compilation ##################


# Add Glint libraries
add_subdirectory(src/glint_core)
add_subdirectory(src/glint_ui)
# add_subdirectory(src/glint_renderer)

# Sample browser
add_subdirectory(src/samples)
 
add_subdirectory(src/minimal)

#TODO: Add tests