
set(PIL_NAME "${PROJECT_NAME}_pil")

set(PIL_SOURCES 
  "src/darray.c"
  "src/darray.h"
  "src/phashtable.c"
  "src/phashtable.h"
  "src/pool_allocator.c"
  "src/pool_allocator.h"
  "src/shared_memory.c"
  "src/shared_memory.h"
  "src/stack_allocator.c"
  "src/stack_allocator.h"
  "src/thread.c"
  "src/thread.h"
)

add_library(${PIL_NAME} ${PIL_SOURCES})

target_include_directories(${PIL_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )

