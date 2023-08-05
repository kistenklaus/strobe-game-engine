#ifndef STROBE_RENDERER_SCENE_MANAGER_H
#define STROBE_RENDERER_SCENE_MANAGER_H

#include "allocators.h"
#include "containers.h"
#include "mat.h"
#include "scene_renderer.h"
#include "threading.h"
#include "profiler.h"

typedef struct {
  unsigned int frameCount;
#ifdef STROBE_RENDERER_ENABLE_PROFILING
  frame_profiler* frameProfiler;
#endif
} scene_manager_create_info;

typedef struct {
  pool_allocator allocator; // of scene_control_block
  darray idLookup;          // of scene_control_block*
  mutex mutex;
  darray* createCommandQueue;
  darray* destroyCommandQueue; // of scene_destroy_command
  unsigned int frameCount;
  darray* orderings;
#ifdef STROBE_RENDERER_ENABLE_PROFILING
  frame_profiler* frameProfiler;
#endif
} scene_manager;

typedef struct {
  scene_renderer_create_info renderer;
  unsigned int layer;
} scene_create_info;

void renderer_scene_manager_init(scene_manager *manager,
                                 scene_manager_create_info *createInfo);

void renderer_scene_manager_create_scene(scene_manager *manager,
                                         unsigned int frame,
                                         unsigned int *scene, scene_create_info* createInfo);

void renderer_scene_manager_destroy_scene(scene_manager *manager,
                                          unsigned int frame,
                                          unsigned int scene);

void renderer_scene_manager_destroy(scene_manager *manager);

void renderer_scene_manager_submit_sof(scene_manager *manager,
                                       unsigned int frame);

void renderer_scene_manager_submit_eof(scene_manager *manager,
                                       unsigned int frame);

void renderer_scene_manager_render_frame(scene_manager* manager, 
    unsigned int frame);

void renderer_scene_manager_submit_drawable(scene_manager *manager,
                                            unsigned int frame,
                                            unsigned int mesh,
                                            unsigned int material,
                                            mat4f *transform);

#endif
