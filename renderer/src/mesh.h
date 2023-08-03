#ifndef STROBE_RENDERER_API_MESH_H
#define STROBE_RENDERER_API_MESH_H

#include "render_backend.h"
#include "render_context.h"

int renderer_mesh_attribute_create_info_is_null(
    mesh_attribute_create_info *attributeCreateInfo);

int renderer_create_mesh(render_context_t *context, unsigned int *mesh,
                         mesh_create_info *meshCreateInfo);

int renderer_destroy_mesh(render_context_t *context, unsigned int mesh);

int renderer_map_mesh_attribute(render_context_t *context, unsigned int mesh,
                                mesh_attribute_type attributeType,
                                mesh_mapped_attribute *attribute);

int renderer_unmap_mesh_attribute(render_context_t *context,
                                mesh_mapped_attribute *attribute);

int renderer_set_mapped_mesh_attribute_vertex(mesh_mapped_attribute *attribute,
                                              size_t vertexIndex, void *data);

int renderer_set_mapped_mesh_attribute_range(mesh_mapped_attribute *attribute,
                                             size_t beginVertex, void *data,
                                             size_t vertexCount);

#endif
