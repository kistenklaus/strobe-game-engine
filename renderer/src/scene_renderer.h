#ifndef STROBE_RENDERER_SCENE_RENDERER_H
#define STROBE_RENDERER_SCENE_RENDERER_H

#include "mat.h"

typedef struct scene_renderer {
  void *body;
  void (*init)(struct scene_renderer *renderer);
  void (*setup)(struct scene_renderer *renderer);
  void (*render)(struct scene_renderer *renderer);
  void (*draw)(struct scene_renderer *renderer, unsigned int mesh,
               unsigned int material, mat4f *transform);
  void (*destroy)(struct scene_renderer *renderer);
} scene_renderer;

typedef struct {
  void (*constructor)(scene_renderer *renderer);
} scene_renderer_create_info;

#endif
