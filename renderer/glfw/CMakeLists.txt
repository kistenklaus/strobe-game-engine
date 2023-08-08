

set(STROBE_GLFW_SRC 
  src/glfw_window.c
  src/glfw_window.h
)

add_library(strobe_glfw ${STROBE_GLFW_SRC})

target_link_libraries(strobe_glfw 
  PRIVATE glfw 
  PUBLIC strobe_renderer)

target_include_directories(strobe_glfw PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
