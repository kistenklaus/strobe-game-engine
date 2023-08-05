
#include "scene.h"
#include "scene_manager.h"

void renderer_create_scene(render_context_t* context, unsigned int* scene, scene_create_info* createInfo){
  renderer_scene_manager_create_scene(&context->sceneManager, context->submitFrame, scene, createInfo);
}

void renderer_destroy_scene(render_context_t* context, unsigned int scene){
  renderer_scene_manager_destroy_scene(&context->sceneManager, context->submitFrame, scene);
}
