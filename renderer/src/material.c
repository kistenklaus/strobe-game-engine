#include "material.h"
#include "material_manager.h"

int renderer_create_material(render_context_t *context, unsigned int *material,
                             material_create_info *createInfo) {
  return renderer_material_manager_create_material(
      &context->materialManager, context->submitFrame, material, createInfo);
}

int renderer_destroy_material(render_context_t *context,
                              unsigned int material) {
  return renderer_material_manager_destroy_material(
      &context->materialManager, context->submitFrame, material);
}
