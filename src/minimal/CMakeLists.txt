

function(add_glint_app APP_NAME)
    add_executable(${APP_NAME} ${APP_NAME}/main.cpp)
    
    target_link_libraries(${APP_NAME} PRIVATE
        glint_core
        glint_ui
    )
    
    # Copy any needed assets to the build directory
    # file(COPY ${SAMPLE_NAME}/assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
    
    # Set output directory
    set_target_properties(${APP_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        FOLDER "MinimalApps"
    )
endfunction()


add_glint_app(triangle)
add_glint_app(cube)
