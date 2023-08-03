#include "mesh.h"

int mesh_attribute_create_info_is_null(
    mesh_attribute_create_info *attributeCreateInfo) {
  return shared_memory_is_null(attributeCreateInfo->data);
}

int renderer_create_mesh(render_context_t *context, unsigned int *mesh,
                         mesh_create_info *meshCreateInfo){
  return context->backend.create_mesh(context->submitFrame, mesh, meshCreateInfo);
}

int renderer_destroy_mesh(render_context_t *context, unsigned int mesh){

  return context->backend.destroy_mesh(context->submitFrame, mesh);
}

int renderer_map_mesh_attribute(render_context_t *context, unsigned int mesh,
                                mesh_attribute_type attributeType,
                                mesh_mapped_attribute *attribute){
  return context->backend.map_mesh_attribute(context->submitFrame, mesh, attributeType, attribute);
}

int renderer_unmap_mesh_attribute(render_context_t *context,
                                mesh_mapped_attribute *attribute){ 
  return context->backend.unmap_mesh_attribute(context->submitFrame, attribute);
}

int renderer_set_mapped_mesh_attribute_vertex(mesh_mapped_attribute *attribute,
                                              size_t vertexIndex, void *data){
  //TODO implement logic here.
  return -1;
}

int renderer_set_mapped_mesh_attribute_range(mesh_mapped_attribute *attribute,
                                             size_t beginVertex, void *data,
                                             size_t vertexCount){
  //TODO implement logic here.
  return -1;
}
