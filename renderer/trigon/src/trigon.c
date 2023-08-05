
#include "trigon.h"
#include <stddef.h>

static void trigon_init(scene_renderer* renderer){
}
static void trigon_setup(scene_renderer* renderer){
}
static void trigon_render(scene_renderer* renderer){
}

static void trigon_draw(scene_renderer* renderer, unsigned int mesh, unsigned int material,
    mat4f* trasform){
}
static void trigon_destroy(scene_renderer* renderer){
}

static void trigon_constructor(scene_renderer* renderer){
  renderer->init = trigon_init;
  renderer->setup = trigon_setup;
  renderer->draw = trigon_draw;
  renderer->render = trigon_render;
  renderer->destroy = trigon_destroy;
  renderer->body = NULL;
}

scene_renderer_create_info TRIGON_SCENE_RENDERER = {
  .constructor = trigon_constructor
};
