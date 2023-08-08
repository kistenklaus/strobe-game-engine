######################################################################
# @author      : karl (karlsasssie@gmail.com)
# @file        : CMakeLists
# @created     : Friday Apr 07, 2023 15:39:05 CEST
######################################################################

add_subdirectory(trigon)


set(STROBE_RENDERER_SRC 
  "src/frame_barrier.c"
  "src/frame_barrier.h"
  "src/frame_guard.c"
  "src/frame_guard.h"
  "src/material.c"
  "src/material.h"
  "src/material_manager.c"
  "src/material_manager.h"
  "src/mesh.c"
  "src/mesh.h"
  "src/mesh_manager.c"
  "src/mesh_manager.h"
  "src/render_context.c"
  "src/render_context.h"
  "src/render_loop.c"
  "src/render_loop.h"
  "src/render_thread.c"
  "src/render_thread.h"
  "src/renderer_submission.c"
  "src/renderer_submission.h"
  "src/scene.c"
  "src/scene.h"
  "src/scene_manager.c"
  "src/scene_manager.h"
  "src/scene_renderer.h"
)

set(RENDERER_NAME "${PROJECT_NAME}_renderer")

set(LIB_NAME "${RENDERER_NAME}")

add_library(${LIB_NAME} ${STROBE_RENDERER_SRC})
target_include_directories(${LIB_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src" #remove later.
  )
target_link_libraries(${LIB_NAME} 
  PUBLIC strobe_pil strobe_lia strobe_logging strobe_profiler)

target_compile_definitions(${RENDERER_NAME} PUBLIC STROBE_RENDERER_ENABLE_PROFILING)

add_subdirectory(glfw)
add_subdirectory(opengl-backend)
add_subdirectory(sandbox)

#create_test_target(${LIB_NAME} test)
