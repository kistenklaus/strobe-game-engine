
set(PROFILER_SRC
  "src/profiler.c"
  "src/profiler.h"
)

add_library(strobe_profiler ${PROFILER_SRC})

target_link_libraries(strobe_profiler PUBLIC strobe_pil)

target_include_directories(strobe_profiler 
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
