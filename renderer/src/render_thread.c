#include "render_thread.h"
#include "frame_barrier.h"
#include "frame_guard.h"
#include "thread.h"
#include "window.h"

static int shouldClose = 0;

static void *render_thread_entry(void *context_arg) {
  render_context_t *context = context_arg;

  window_t *window = context->window;
  window->init(window);

  render_backend* backend = &context->backend;
  unsigned int frameCount = context->frameGuard.frameCount; 
  backend->init(&frameCount, frameCount); //force frameCount if swapBufferCount != 0.
  if(!frameCount)frameCount = context->frameGuard.frameCount;

  frame_guard_init(&context->frameGuard, frameCount);

  frame_barrier_complete_wait(&context->frameBarrier);

  unsigned int frame;
  while (!(shouldClose || window->shouldClose)) {
    frame_guard_acquire_complete_frame(&context->frameGuard, &context->renderFrame);

    window->poll_events(window);

    context->backend.display(frame);

    window->swap_buffers(window);

    frame_guard_release_complete_frame(&context->frameGuard);
  }

  window->destroy(window);

  frame_barrier_complete_wait(&context->frameBarrier);

  return NULL;
}

void render_thread_start(render_context_t *context) {
  thread_create(&(context->renderThread), render_thread_entry, context);
  frame_barrier_submit_wait(&context->frameBarrier);
}

void render_thread_join(render_context_t *context) {
  shouldClose = 1;
  frame_guard_release_submit_frame(&context->frameGuard);
  frame_barrier_submit_wait(&context->frameBarrier);
  thread_join(&context->renderThread, NULL);
}


