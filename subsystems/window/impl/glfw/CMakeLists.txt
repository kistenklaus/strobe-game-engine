
set(STROBE_WINDOW_GLFW_IMPL ${STROBE_WINDOW_IMPL}-glfw CACHE INTERNAL "strobe window glfw implementation <target>")

add_library(${STROBE_WINDOW_GLFW_IMPL}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/window/WindowImpl.cpp
)
target_link_libraries(${STROBE_WINDOW_GLFW_IMPL} PUBLIC ${STROBE_WINDOW_IMPL})

target_link_glfw(${STROBE_WINDOW_GLFW_IMPL} PRIVATE)
