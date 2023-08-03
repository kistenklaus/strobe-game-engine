#include "render_context.h"
#include "frame_barrier.h"
#include "frame_guard.h"
#include "render_thread.h"
#include <stdlib.h>
#include "phashtable.h"

void render_context_init(render_context_t *context,
                         render_context_create_info *createInfo) {
  context->materialCount = 0;
  context->materialDescriptions = NULL;
  context->materialPropertyDescriptions = NULL;
  context->window = createInfo->window;
  createInfo->backend.create_backend(&context->backend);
  context->frameGuard.frameCount = createInfo->swapBufferCount;
  frame_barrier_init(&context->frameBarrier);

  render_thread_start(context);
}

void render_context_destroy(render_context_t *context) {
    free(context->materialDescriptions);
    if(context->materialPropertyDescriptions != NULL){
      for(unsigned int i = 0; i< context->materialCount; i++){
        free(context->materialPropertyDescriptions[i]);
      }
      free(context->materialPropertyDescriptions);
    }
}
