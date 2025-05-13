

add_executable(strobe-sandbox 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_glfw(strobe-sandbox PUBLIC)
