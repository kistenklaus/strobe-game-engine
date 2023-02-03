
add_executable(opengl_render_backend_testing
        main.cpp)

target_link_libraries(opengl_render_backend_testing PRIVATE ${PROJECT_NAME}_render_backend)