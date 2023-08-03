

set(COMMON_SOURCES 
  "src/window.h"
  "src/render_backend.c"
  "src/render_backend.h"
)

add_library(strobe_renderer_common ${COMMON_SOURCES})

target_link_libraries(strobe_renderer_common PUBLIC strobe_pil strobe_lia)

set_target_properties(strobe_renderer_common PROPERTIES LINKER_LANGUAGE C)

target_include_directories(strobe_renderer_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
