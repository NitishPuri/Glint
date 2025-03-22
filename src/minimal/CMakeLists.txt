# Common sample properties
# function(add_glint_sample SAMPLE_NAME)
#     add_executable(${SAMPLE_NAME} ${SAMPLE_NAME}/main.cpp)
    
#     target_link_libraries(${SAMPLE_NAME} PRIVATE
#         glint_core
#         glint_imgui
#     )
    
#     # Copy any needed assets to the build directory
#     # file(COPY ${SAMPLE_NAME}/assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
    
#     # Set output directory
#     set_target_properties(${SAMPLE_NAME} PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#     )
# endfunction()

# # Add individual samples
# add_glint_sample(triangle)
# add_glint_sample(cube)
# # Add more samples as needed