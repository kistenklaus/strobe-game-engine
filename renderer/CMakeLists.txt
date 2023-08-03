######################################################################
# @author      : karl (karlsasssie@gmail.com)
# @file        : CMakeLists
# @created     : Friday Apr 07, 2023 15:39:05 CEST
######################################################################

add_subdirectory(common)


set(STROBE_RENDERER_SRC 
  "src/frame_barrier.c"
  "src/frame_barrier.h"
  "src/frame_guard.c"
  "src/frame_guard.h"
  "src/material.c"
  "src/material.h"
  "src/mesh.c"
  "src/mesh.h"
  "src/render_context.c"
  "src/render_context.h"
  "src/render_loop.c"
  "src/render_loop.h"
  "src/render_thread.c"
  "src/render_thread.h"
  "src/renderer_submission.c"
  "src/renderer_submission.h"
)

set(RENDERER_NAME "${PROJECT_NAME}_renderer")

set(LIB_NAME "${RENDERER_NAME}")

add_library(${LIB_NAME} ${STROBE_RENDERER_SRC})
target_include_directories(${LIB_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src" #remove later.
  )
target_link_libraries(${LIB_NAME} 
  PUBLIC strobe_pil strobe_renderer_common strobe_lia)

add_subdirectory(glfw)
add_subdirectory(trigon)
add_subdirectory(sandbox)

#create_test_target(${LIB_NAME} test)
