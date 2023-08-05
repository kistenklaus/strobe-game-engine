#ifndef STROBE_RENDERER_API_MESH_H
#define STROBE_RENDERER_API_MESH_H

#include "render_backend.h"
#include "render_context.h"

int renderer_mesh_attribute_create_info_is_null(
    mesh_attribute_create_info *attributeCreateInfo);

int renderer_create_mesh(render_context_t *context, unsigned int *mesh,
                         mesh_create_info *meshCreateInfo);

int renderer_destroy_mesh(render_context_t *context, unsigned int mesh);


#endif
