

set(LOGGING_SRC 
  "src/log.c"
  "src/log.h"
)

add_library(strobe_logging ${LOGGING_SRC})

target_compile_definitions(strobe_logging PUBLIC 
  STROBE_LOG_FATAL_ERRORS
  STROBE_LOG_ERRORS
  STROBE_LOG_WARNINGS
  STROBE_LOG_INFOS
  STROBE_LOG_DEBUG
  STROBE_LOG_STACK_TRACE
)

target_link_libraries(strobe_logging strobe_pil)
target_include_directories(strobe_logging PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")



