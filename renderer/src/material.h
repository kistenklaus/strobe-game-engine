#ifndef STROBE_RENDERER_MATERIAL_H
#define STROBE_RENDERER_MATERIAL_H

#include "render_context.h"

int renderer_create_material(render_context_t* context, unsigned int* material, 
    material_create_info* createInfo);

int renderer_destroy_material(render_context_t* context, unsigned int material);


#endif
