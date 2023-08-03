#include "renderer_submission.h"


int renderer_draw(render_context_t* context, unsigned int mesh, unsigned int material, 
    mat4f* transform){
  return context->backend.draw(context->submitFrame, mesh, material, transform);
}
