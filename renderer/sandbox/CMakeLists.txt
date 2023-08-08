

set(SANDBOX_SOURCES 
  src/main.c
)

set(SANDBOX_EXEC_NAME "${RENDERER_NAME}_sandbox")

add_executable(${SANDBOX_EXEC_NAME} ${SANDBOX_SOURCES})

# target_link_libraries(${SANDBOX_EXEC_NAME} PUBLIC "${LIB_NAME}")
target_link_libraries(${SANDBOX_EXEC_NAME} 
  PUBLIC 
    "${LIB_NAME}" 
    strobe_glfw 
    strobe_trigon 
    strobe_opengl_backend
)


