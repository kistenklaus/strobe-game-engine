

set(TRIGON_SOURCES 
  "src/buffer.c"
  "src/buffer.h"
  "src/instance.c"
  "src/instance.h"
  "src/link.c"
  "src/material_manager.c"
  "src/material_manager.h"
  "src/mesh_manager.c"
  "src/mesh_manager.h"
  "src/submission.c"
  "src/submission.h"
)

set(TRIGON_NAME "strobe_renderer_trigon")

add_library(${TRIGON_NAME} ${TRIGON_SOURCES})

target_link_libraries(${TRIGON_NAME} PUBLIC strobe_pil strobe_renderer_common)

target_include_directories(${TRIGON_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
