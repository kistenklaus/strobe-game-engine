#ifndef STROBE_RENDERER_RENDER_CONTEXT_H
#define STROBE_RENDERER_RENDER_CONTEXT_H

#include "frame_barrier.h"
#include "scene_manager.h"
#include "threading.h"
#include "frame_guard.h"
#include "window.h"
#include "render_backend.h"
#include "containers.h"
#include "allocators.h"
#include "mesh_manager.h"
#include "material_manager.h"
#include "timeutil.h"
#include "profiler.h"


typedef struct {
  thread renderThread;
  frame_guard_t frameGuard;
  frame_barrier_t frameBarrier;
  window_t* window;
  unsigned int renderFrame;
  unsigned int submitFrame;
  render_backend backend;

  mesh_manager meshManager;
  material_manager materialManager;
  scene_manager sceneManager;

  int shouldClose;
  int running;
  int started;
#ifdef STROBE_RENDERER_ENABLE_PROFILING
  frame_profiler frameProfiler;
#endif
  darray requiredDestroyCalls; // function calls.
} render_context_t;


typedef struct {
  unsigned int swapBufferCount;
  window_t* window;
  render_backend_create_info backend;
} render_context_create_info;

void render_context_init(render_context_t* context, render_context_create_info* createInfo);

int renderer_start(render_context_t* context);

void renderer_stop(render_context_t* context);

void render_context_destroy(render_context_t*);


#endif
