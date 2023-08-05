

set(TRIGON_SRC
  "src/trigon.c"
  "src/trigon.h"
)

add_library(strobe_trigon ${TRIGON_SRC})

target_link_libraries(strobe_trigon PUBLIC strobe_renderer)
target_include_directories(strobe_trigon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
