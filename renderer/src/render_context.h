#ifndef STROBE_RENDERER_RENDER_CONTEXT_H
#define STROBE_RENDERER_RENDER_CONTEXT_H

#include "frame_barrier.h"
#include "thread.h"
#include "frame_guard.h"
#include "window.h"
#include "render_backend.h"
#include "phashtable.h"

typedef struct {
  thread_t renderThread;
  frame_guard_t frameGuard;
  frame_barrier_t frameBarrier;
  window_t* window;
  unsigned int renderFrame;
  unsigned int submitFrame;
  render_backend backend;

  unsigned int materialCount;
  material_description* materialDescriptions;
  material_property_description** materialPropertyDescriptions;
} render_context_t;


typedef struct {
  unsigned int swapBufferCount;
  window_t* window;
  render_backend_create_info backend;
} render_context_create_info;

void render_context_init(render_context_t* context, render_context_create_info* createInfo);

void render_context_destroy(render_context_t*);

#endif

