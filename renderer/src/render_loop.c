#include "render_loop.h"
#include "frame_guard.h"

void renderer_begin_frame(render_context_t *context) {
  frame_guard_acquire_submit_frame(&context->frameGuard, &context->submitFrame);
}

int renderer_is_running(render_context_t* context){
  return context->running;
}

void renderer_end_frame(render_context_t *context) {
  frame_guard_release_submit_frame(&context->frameGuard);
}
