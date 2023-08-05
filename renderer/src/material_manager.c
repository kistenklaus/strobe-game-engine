
#include "material_manager.h"
#include "render_backend.h"
#include <stdlib.h>

typedef struct {

} material_control_block;

typedef struct {
  material_control_block* controlBlock;
  material_create_info createInfo;
} material_create_command;

typedef struct {
  material_control_block* controlBlock;
  
} material_update_command;

typedef struct {
  material_control_block* controlBlock;
  unsigned int id;
} material_destroy_command;

int renderer_material_manager_init(material_manager *manager,
                                   material_manager_create_info *createInfo) {
  pool_allocator_create(&manager->allocator, sizeof(material_control_block), 128);

  darray_create(&manager->idLookup, sizeof(material_control_block*), 128);
  mutex_init(&manager->idLookupMutex);

  manager->frameCount = createInfo->frameCount;

  manager->createCommandQueues = calloc(manager->frameCount, sizeof(darray));
  manager->updateCommandQueues = calloc(manager->frameCount, sizeof(darray));
  manager->destroyCommandQueues = calloc(manager->frameCount, sizeof(darray));
  for(unsigned int i = 0; i < manager->frameCount; i++){
    darray_create(manager->createCommandQueues + i, sizeof(material_create_command), 16);
    darray_create(manager->updateCommandQueues + i, sizeof(material_update_command), 16);
    darray_create(manager->destroyCommandQueues + i, sizeof(material_destroy_command), 16);
  }
  return 0;
}

int renderer_material_manager_create_material(
    material_manager *manager, unsigned int frame, unsigned int *material,
    material_create_info *createInfo) {
  // TODO Make createInfo memory shared if necassary!
  
  // Allocate new control block.
  material_control_block* controlBlock = pool_allocator_alloc_element(&manager->allocator);

  // Find new id.
  mutex_lock(&manager->idLookupMutex);
  size_t pos = darray_find(&manager->idLookup, is_null_filter);
  if(pos == darray_size(&manager->idLookup)){
    darray_push_back(&manager->idLookup, &controlBlock);
  }else{
    darray_set(&manager->idLookup, pos, &controlBlock);
  }
  mutex_unlock(&manager->idLookupMutex);

  darray* createQueue = manager->createCommandQueues + frame;
  material_create_command* command = darray_emplace_back(createQueue);
  command->controlBlock = controlBlock;
  command->createInfo = *createInfo;
  return 0;
}

int renderer_material_manager_destroy_material(material_manager *manager,
                                               unsigned int frame, unsigned int material) {
  material_control_block* controlBlock = darray_get(&manager->idLookup, material);
  darray* destroyQueue = manager->destroyCommandQueues + frame;
  material_destroy_command* command = darray_emplace_back(destroyQueue);
  command->controlBlock = controlBlock;
  command->id = material;
  return 0;
}

int renderer_material_manager_prepare_frame(material_manager* manager, unsigned int frame){
  darray* createQueue = manager->createCommandQueues + frame;
  darray_iterator createQueueIt = darray_iterate(createQueue);
  while(darray_iterator_has_next(&createQueueIt)){
    material_create_command* command = darray_iterator_next(&createQueueIt);
    // API CALL!
  }
  darray_clear(createQueue);


  darray* updateQueue = manager->updateCommandQueues + frame;
  darray_iterator updateQueueIt = darray_iterate(updateQueue);
  while(darray_iterator_has_next(&updateQueueIt)){
    material_update_command* command = darray_iterator_next(&updateQueueIt);
    // API CALL!.
  }
  darray_clear(updateQueue);

  darray* destroyQueue = manager->destroyCommandQueues + frame;
  darray_iterator destroyQueueIt = darray_iterate(destroyQueue);
  while(darray_iterator_has_next(&destroyQueueIt)){
    material_destroy_command* command = darray_iterator_next(&destroyQueueIt);
    // API CALL!
    // Invalidate id.
    mutex_lock(&manager->idLookupMutex);
    *((material_control_block**)darray_get(&manager->idLookup, command->id)) = NULL;
    mutex_unlock(&manager->idLookupMutex);
  }
  darray_clear(destroyQueue);

  return 0;
}

int renderer_material_manager_destroy(material_manager *manager) {
  for(unsigned int i = 0; i < manager->frameCount; i++){
    darray_destroy(manager->createCommandQueues + i);
    darray_destroy(manager->updateCommandQueues + i);
    darray_destroy(manager->destroyCommandQueues + i);
  }
  free(manager->createCommandQueues);
  free(manager->updateCommandQueues);
  free(manager->destroyCommandQueues);

  mutex_destroy(&manager->idLookupMutex);
  darray_destroy(&manager->idLookup);

  pool_allocator_destroy(&manager->allocator);
  return 0;
}
