#include "render_context.h"
#include "frame_barrier.h"
#include "frame_guard.h"
#include "mesh_manager.h"
#include "render_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include "containers.h"
#include "mesh.h"

void render_context_init(render_context_t *context,
                         render_context_create_info *createInfo) {

  context->window = createInfo->window;
  context->frameGuard.frameCount = createInfo->swapBufferCount;
  context->submitFrame = 0;
  context->shouldClose = 0;
  context->running = 0;
  context->started = 0;
  createInfo->backend.constructor(&context->backend);
  frame_barrier_init(&context->frameBarrier);
}

int renderer_start(render_context_t* context){
  if(context->started)return 1;
  context->started = 1;
  context->shouldClose = 0;
  unsigned int code = render_thread_start(context);
  if(code){
    context->started = 0;
  }
  return code;
}

void renderer_stop(render_context_t* context){
  if(!context->started)return;
  context->shouldClose = 1;
  context->started = 0;
  render_thread_join(context);
}

void render_context_destroy(render_context_t *context) {
}
