#include "mesh.h"

int mesh_attribute_create_info_is_null(
    mesh_attribute_create_info *attributeCreateInfo) {
  return shared_memory_is_null(attributeCreateInfo->data);
}

int renderer_create_mesh(render_context_t *context, unsigned int *mesh,
                         mesh_create_info *meshCreateInfo){
  return renderer_mesh_manager_create_mesh(&context->meshManager, context->submitFrame, mesh, meshCreateInfo);
}

int renderer_destroy_mesh(render_context_t *context, unsigned int mesh){
  return renderer_mesh_manager_destroy_mesh(&context->meshManager, context->submitFrame, mesh);
}
