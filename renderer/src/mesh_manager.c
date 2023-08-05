#include "mesh_manager.h"
#include "containers.h"
#include "log.h"
#include "threading.h"
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  
} mesh_control_block;

typedef struct {
  mesh_control_block* controlBlock;
  mesh_create_info createInfo;
} mesh_create_command;

typedef struct {
  mesh_control_block* controlBlock;
  mesh_update_info updateInfo;
} mesh_update_command;

typedef struct {
  mesh_control_block* controlBlock;
  unsigned int id;
} mesh_destroy_command;

int renderer_mesh_manager_init(mesh_manager* manager, mesh_manager_create_info* createInfo){
  pool_allocator_create(&manager->allocator, sizeof(mesh_control_block), 128);

  darray_create(&manager->idLookup, sizeof(mesh_control_block*), 128);
  mutex_init(&manager->idLookupMutex);

  manager->frameCount = createInfo->frameCount;

  manager->createCommandQueues = calloc(manager->frameCount, sizeof(darray));
  manager->updateCommandQueues = calloc(manager->frameCount, sizeof(darray));
  manager->destroyCommandQueues = calloc(manager->frameCount, sizeof(darray));

  for(unsigned int i = 0; i < manager->frameCount; i++){
    darray_create(manager->createCommandQueues + i, sizeof(mesh_create_command), 16);
    darray_create(manager->updateCommandQueues + i, sizeof(mesh_update_command), 16);
    darray_create(manager->destroyCommandQueues + i, sizeof(mesh_destroy_command), 16);
  }
  return 0;
}


int renderer_mesh_manager_create_mesh(mesh_manager* manager, unsigned int frame, unsigned int* mesh, mesh_create_info* createInfo){

  // Make memory of create info shared.
  if(!shared_memory_is_null(createInfo->positions.data)){
    shared_memory_make_shared(createInfo->positions.data);
  }
  if(!shared_memory_is_null(createInfo->textureCoords.data)){
    shared_memory_make_shared(createInfo->textureCoords.data);
  }
  if(!shared_memory_is_null(createInfo->normals.data)){
    shared_memory_make_shared(createInfo->normals.data);
  }
  if(!shared_memory_is_null(createInfo->tangents.data)){
    shared_memory_make_shared(createInfo->tangents.data);
  }
  if(!shared_memory_is_null(createInfo->bitangents.data)){
    shared_memory_make_shared(createInfo->bitangents.data);
  }
  if(!shared_memory_is_null(createInfo->colors.data)){
    shared_memory_make_shared(createInfo->colors.data);
  }

  // Allocate new control block.
  mesh_control_block* controlBlock = pool_allocator_alloc_element(&manager->allocator);
  
  // Find new id.
  mutex_lock(&manager->idLookupMutex);
  size_t pos = (unsigned int)darray_find(&manager->idLookup, is_null_filter);
  if(pos == darray_size(&manager->idLookup)){
    darray_push_back(&manager->idLookup, &controlBlock);
  }else{
    darray_set(&manager->idLookup, pos, &controlBlock);
  }
  mutex_unlock(&manager->idLookupMutex);


  darray* createQueue = manager->createCommandQueues + frame;
  mesh_create_command* command = darray_emplace_back(createQueue);
  command->controlBlock = controlBlock;
  command->createInfo = *createInfo;


  return 0;
}

int renderer_mesh_manager_destroy_mesh(mesh_manager* manager, unsigned int frame, unsigned int mesh){
  // Resolve Id.
  mesh_control_block* controlBlock = darray_get(&manager->idLookup, mesh);

  darray* destroyQueue = manager->destroyCommandQueues + frame;
  mesh_destroy_command* command = darray_emplace_back(destroyQueue);
  command->controlBlock = controlBlock;
  command->id = mesh;

  return 0;
}

int renderer_mesh_manager_update_mesh(mesh_manager* manager, unsigned int frame, unsigned int mesh, mesh_update_info* updateInfo){
  mesh_control_block* controlBlock = darray_get(&manager->idLookup, mesh);
  darray* updateQueue = manager->updateCommandQueues + frame;
  mesh_update_command* command = darray_emplace_back(updateQueue);
  command->controlBlock = controlBlock;
  command->updateInfo = *updateInfo;
  return 0;
}

int renderer_mesh_manager_prepare_frame(mesh_manager* manager, unsigned int frame){

  darray* createQueue = manager->createCommandQueues + frame;
  darray_iterator createQueueIt = darray_iterate(createQueue);
  while(darray_iterator_has_next(&createQueueIt)){
    mesh_create_command* command = darray_iterator_next(&createQueueIt);
    // BACKEND-CALL!
  }
  darray_clear(createQueue);

  darray* updateQueue = manager->updateCommandQueues + frame;
  darray_iterator updateQueueIt = darray_iterate(updateQueue);
  while(darray_iterator_has_next(&updateQueueIt)){
    mesh_update_command* command = darray_iterator_next(&updateQueueIt);
    // BACKEND CALL!
  }
  darray_clear(updateQueue);

  darray* destroyQueue = manager->destroyCommandQueues + frame;
  darray_iterator destroyQueueIt = darray_iterate(destroyQueue);
  while(darray_iterator_has_next(&destroyQueueIt)){
    mesh_destroy_command* command = darray_iterator_next(&destroyQueueIt);
    // BACKEND CALL!
    // Invalidate id.
    mutex_lock(&manager->idLookupMutex);
    *((mesh_control_block**)darray_get(&manager->idLookup, command->id)) = NULL;
    mutex_unlock(&manager->idLookupMutex);
  }
  darray_clear(destroyQueue);
  
  return 0;
}

int renderer_mesh_manager_destroy(mesh_manager* manager){
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

