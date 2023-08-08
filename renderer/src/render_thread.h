#ifndef STROBE_RENDERER_RENDER_THREAD_H
#define STROBE_RENDERER_RENDER_THREAD_H

#include "render_context.h"

int render_thread_start(render_context_t* context);

void render_thread_join(render_context_t* context);

#endif

