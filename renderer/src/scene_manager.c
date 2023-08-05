#include "scene_manager.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef STROBE_RENDERER_ENABLE_PROFILING
static void PROFILER_PUSH_BLOCK(scene_manager *manager, char *blockName) {
  // frame_profiler_push_block(manager->frameProfiler, blockName);
}
static void PROFILER_POP_BLOCK(scene_manager *manager) {
  // frame_profiler_pop_block(manager->frameProfiler);
}
#else
static void PROFILER_PUSH_BLOCK(scene_manager *manager, char *blockName) {}
static void PROFILER_POP_BLOCK(scene_manager *manager) {}
#endif

static int cmp_int(const void *va, const void *vb) {
  int a = *(int *)va, b = *(int *)vb;
  return a < b ? -1 : a > b ? +1 : 0;
}

typedef struct {
  scene_renderer* sceneRenderers;
  unsigned int layer;
} scene_control_block;

typedef struct {
  scene_control_block *controlBlock;
  unsigned int id;
} scene_destroy_command;

typedef struct {
  scene_control_block *controlBlock;
  scene_create_info createInfo;
} scene_create_command;

void renderer_scene_manager_init(scene_manager *manager,
                                 scene_manager_create_info *createInfo) {
  pool_allocator_create(&manager->allocator, sizeof(scene_control_block), 16);

  darray_create(&manager->idLookup, sizeof(scene_control_block *), 16);

  mutex_init(&manager->mutex);

  manager->frameCount = createInfo->frameCount;
  manager->createCommandQueue = calloc(manager->frameCount, sizeof(darray));
  manager->destroyCommandQueue = calloc(manager->frameCount, sizeof(darray));
  for (unsigned int i = 0; i < manager->frameCount; i++) {
    darray_create(manager->createCommandQueue + i, sizeof(scene_create_command),
                  4);
    darray_create(manager->destroyCommandQueue + i,
                  sizeof(scene_destroy_command), 4);
  }

  manager->orderings = calloc(manager->frameCount, sizeof(darray));
  for (unsigned int i = 0; i < manager->frameCount; i++) {
    darray_create(manager->orderings + i, sizeof(unsigned int), 4);
  }

#ifdef STROBE_RENDERER_ENABLE_PROFILING
  manager->frameProfiler = createInfo->frameProfiler;
#endif
}

void renderer_scene_manager_create_scene(scene_manager *manager,
                                         unsigned int frame,
                                         unsigned int *scene,
                                         scene_create_info *createInfo) {
  mutex_lock(&manager->mutex);
  scene_control_block *controlBlock =
      pool_allocator_alloc_element(&manager->allocator);

  size_t pos = darray_find(&manager->idLookup, is_null_filter);
  if (pos == darray_size(&manager->idLookup)) {
    darray_push_back(&manager->idLookup, &controlBlock);
  } else {
    darray_set(&manager->idLookup, pos, &controlBlock);
  }
  *scene = pos;
  mutex_unlock(&manager->mutex);

  darray *createQueue = manager->createCommandQueue + frame;
  scene_create_command *command = darray_emplace_back(createQueue);
  command->controlBlock = controlBlock;
  command->createInfo = *createInfo;

  // Allocate required resources for render graph submission.
  controlBlock->sceneRenderers = calloc(manager->frameCount, sizeof(scene_renderer));
  for (unsigned int i = 0; i < manager->frameCount; i++) {
    scene_renderer* renderer = controlBlock->sceneRenderers + i;
    createInfo->renderer.constructor(renderer);
    // Allocation is not initalization; it doesn't require a backend context!
    // But it ensures that submissions can be made to the rendergraph.
    renderer->init(renderer);
  }
  return;

  controlBlock->layer = createInfo->layer;
  darray *ordering = manager->orderings + frame;
  *(unsigned int *)darray_emplace_back(ordering) = pos;
  darray_sort(ordering, cmp_int);
}

void renderer_scene_manager_destroy_scene(scene_manager *manager,
                                          unsigned int frame,
                                          unsigned int scene) {

  // only mark for deletion.
  darray *destroyQueue = manager->destroyCommandQueue + frame;
  scene_destroy_command *command = darray_emplace_back(destroyQueue);
  command->controlBlock = darray_get(&manager->idLookup, scene);
  command->id = scene;
}

void renderer_scene_manager_destroy(scene_manager *manager) {
  for (unsigned int i = 0; i < manager->frameCount; i++) {
    darray_destroy(manager->orderings + i);
  }
  free(manager->orderings);

  for (unsigned int i = 0; i < manager->frameCount; i++) {
    darray_destroy(manager->destroyCommandQueue + i);
    darray_destroy(manager->createCommandQueue + i);
  }
  free(manager->destroyCommandQueue);
  free(manager->createCommandQueue);
  mutex_destroy(&manager->mutex);
  darray_destroy(&manager->idLookup);
  pool_allocator_destroy(&manager->allocator);
}

void renderer_scene_manager_submit_sof(scene_manager *manager,
                                       unsigned int frame) {}

void renderer_scene_manager_submit_eof(scene_manager *manager,
                                       unsigned int frame) {}

void renderer_scene_manager_render_frame(scene_manager *manager,
                                         unsigned int frame) {
  // Initalize render graphs before rendering.
  PROFILER_PUSH_BLOCK(manager, "scene-manager-create-scenes");
  darray *createQueue = manager->createCommandQueue + frame;
  darray_iterator createQueueIt = darray_iterate(createQueue);
  while (darray_iterator_has_next(&createQueueIt)) {
    scene_create_command *command = darray_iterator_next(&createQueueIt);
    for (unsigned int i = 0; i < manager->frameCount; i++) {
      scene_renderer* renderer = command->controlBlock->sceneRenderers + i;
      renderer->setup(renderer);
    }
  }
  darray_clear(createQueue);
  PROFILER_POP_BLOCK(manager);

  return;
  // Execute render graph.
  PROFILER_PUSH_BLOCK(manager, "render-layers");
  darray *ordering = manager->orderings + frame;
  darray_iterator orderingIt = darray_iterate(ordering);
  while (darray_iterator_has_next(&orderingIt)) {
    unsigned int *sceneId = darray_iterator_next(&orderingIt);
    scene_control_block **controlBlock =
        darray_get(&manager->idLookup, *sceneId);

#ifdef STROBE_RENDERER_ENABLE_PROFILING
    static char fmtStr[200];
    sprintf(fmtStr, "render-scene [id = %u]", *sceneId);
#else
    static char *fmtStr = NULL;
#endif

    PROFILER_PUSH_BLOCK(manager, fmtStr);
    scene_renderer* renderer = (*controlBlock)->sceneRenderers + frame;
    renderer->render(renderer);
    PROFILER_POP_BLOCK(manager);
  }
  PROFILER_POP_BLOCK(manager);

  // Handle scene destroy commands.
  PROFILER_PUSH_BLOCK(manager, "scene-manager-destroy-scenes");
  darray *destroyQueue = manager->destroyCommandQueue + frame;
  darray_iterator destroyQueueIt = darray_iterate(destroyQueue);
  while (darray_iterator_has_next(&destroyQueueIt)) {
    scene_destroy_command *command = darray_iterator_next(&destroyQueueIt);
    for (unsigned int i = 0; i < manager->frameCount; i++) {
      scene_renderer* renderer = command->controlBlock->sceneRenderers + i;
      renderer->destroy(renderer);
    }
    mutex_lock(&manager->mutex);
    *(scene_control_block **)darray_get(&manager->idLookup, command->id) = NULL;
    pool_allocator_free_element(&manager->allocator, command->controlBlock);
    mutex_unlock(&manager->mutex);
  }
  darray_clear(destroyQueue);
  PROFILER_POP_BLOCK(manager);
}

void renderer_scene_manager_submit_drawable(scene_manager *manager,
                                            unsigned int frame,
                                            unsigned int mesh,
                                            unsigned int material,
                                            mat4f *transform) {}
