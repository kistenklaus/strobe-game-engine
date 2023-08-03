#include "render_loop.h"
#include "frame_guard.h"

void renderer_begin_frame(render_context_t *context) {
  frame_guard_acquire_submit_frame(&context->frameGuard, &context->submitFrame);
}

void renderer_end_frame(render_context_t *context) {
  frame_guard_release_submit_frame(&context->frameGuard);
}
