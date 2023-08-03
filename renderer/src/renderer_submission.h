#ifndef STROBE_RENDERER_RENDERER_SUBMISSION_H
#define STROBE_RENDERER_RENDERER_SUBMISSION_H
#include "render_context.h"
#include "mat.h"

int renderer_draw(render_context_t* context, unsigned int mesh, unsigned int material, 
    mat4f* transform);

#endif
