

set(OPENGL_SRC
  "src/opengl_backend.c"
  "src/opengl_backend.h"
)

add_library(strobe_opengl_backend ${OPENGL_SRC})

target_link_libraries(strobe_opengl_backend 
  PUBLIC 
    strobe_renderer
    strobe_logging
)
target_include_directories(strobe_opengl_backend 
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")


