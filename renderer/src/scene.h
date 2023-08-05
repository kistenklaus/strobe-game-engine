#ifndef STROBE_RENDERER_SCENE_H
#define STROBE_RENDERER_SCENE_H

#include "render_context.h"

void renderer_create_scene(render_context_t* context, unsigned int* scene, scene_create_info* createInfo);

void renderer_destroy_scene(render_context_t* context, unsigned int scene);

#endif

