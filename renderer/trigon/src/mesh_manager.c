#include "mesh_manager.h"
#include "darray.h"
#include "instance.h"
#include "shared_memory.h"
#include "stack_allocator.h"
#include <stdlib.h>
#include <unistd.h>

static const unsigned int INVALID_ID = -1;

static int invalid_id_filter(void *id) {
  return (*(unsigned int *)id) == INVALID_ID;
}

static mesh_control_block *resolve_mesh_id(unsigned int meshId) {
  return darray_get(&trigon_instance.meshManager.idLookup, meshId);
}

int trigon_mesh_manager_init() {
  pool_allocator_create(&trigon_instance.meshManager.allocator,
                        sizeof(mesh_control_block *), 128);

  trigon_instance.meshManager.createCommandQueues =
      calloc(trigon_instance.frameCount, sizeof(darray));
  trigon_instance.meshManager.destroyCommandQueues =
      calloc(trigon_instance.frameCount, sizeof(darray));
  trigon_instance.meshManager.updateCommandQueues =
      calloc(trigon_instance.frameCount, sizeof(darray));
  for (unsigned int i = 0; i < trigon_instance.frameCount; i++) {
    darray_create(trigon_instance.meshManager.createCommandQueues + i,
                  sizeof(mesh_create_command), 32);
    darray_create(trigon_instance.meshManager.destroyCommandQueues + i,
                  sizeof(mesh_destroy_command), 32);
    darray_create(trigon_instance.meshManager.updateCommandQueues + i,
                  sizeof(mesh_update_command), 32);
  }

  return 0;
}

int trigon_create_mesh(unsigned int frame, unsigned int *mesh,
                       mesh_create_info *meshCreateInfo) {
  mesh_control_block *controlBlock =
      pool_allocator_alloc_element(&trigon_instance.meshManager.allocator);

  darray *createCommandQueue =
      trigon_instance.meshManager.createCommandQueues + frame;
  mesh_create_command *createCommand = darray_emplace_back(createCommandQueue);
  createCommand->controlBlock = controlBlock;
  createCommand->createInfo = *meshCreateInfo; // copy!
                                               //
  // find new id.
  darray *idLookup = &trigon_instance.meshManager.idLookup;
  size_t id = darray_find(idLookup, invalid_id_filter);
  if (id == darray_size(idLookup)) {
    darray_push_back(idLookup, &controlBlock);
  } else {
    darray_set(idLookup, id, &controlBlock);
  }
  return id;
}

int trigon_destroy_mesh(unsigned int frame, unsigned int mesh) {
  darray *destroyQueue =
      trigon_instance.meshManager.destroyCommandQueues + frame;
  mesh_destroy_command *command = darray_emplace_back(destroyQueue);
  command->controlBlock = resolve_mesh_id(mesh);
  return 0;
}

int trigon_update_mesh(unsigned int frame, unsigned int mesh, mesh_update_info* updateInfo){
  // Mark all memory as shared to avoid premature deletion.
  shared_memory_make_shared(updateInfo->attributeUpdateInfos);
  mesh_attribute_update_info* attribUpdateInfos = shared_memory_get(updateInfo->attributeUpdateInfos);
  for(unsigned int i = 0; i < updateInfo->attributeUpdateInfoCount; i++){
    shared_memory_make_shared(attribUpdateInfos[i].data);
  }
  darray* updateQueue = trigon_instance.meshManager.updateCommandQueues + frame;
  mesh_update_command* command = darray_emplace_back(updateQueue);
  command->controlBlock = resolve_mesh_id(mesh);
  command->updateInfo = *updateInfo;
  return 0;
}

int trigon_mesh_manager_destroy() {
  pool_allocator_destroy(&trigon_instance.meshManager.allocator);

  for (unsigned int i = 0; i < trigon_instance.frameCount; i++) {
    darray_destroy(trigon_instance.meshManager.createCommandQueues + i);
    darray_destroy(trigon_instance.meshManager.destroyCommandQueues + i);
  }
  free(trigon_instance.meshManager.createCommandQueues);
  free(trigon_instance.meshManager.destroyCommandQueues);
  return 0;
}

int trigon_mesh_manager_prepare_frame(unsigned int frame) {
  // Handle creation of meshes
  darray* createQueue = trigon_instance.meshManager.createCommandQueues + frame;
  darray_iterator createQueueIt = darray_iterate(createQueue);
  while(darray_iterator_has_next(&createQueueIt)){
    mesh_create_command* command = darray_iterator_next(&createQueueIt);
    // BACKEND_CALL!
  }
  darray_clear(createQueue);

  // Handle updates of meshes
  darray* updateQueue = trigon_instance.meshManager.updateCommandQueues + frame;
  darray_iterator updateQueueIt = darray_iterate(updateQueue);
  while(darray_iterator_has_next(&updateQueueIt)){
    mesh_update_command* command = darray_iterator_next(&updateQueueIt);
    // BACKEND CALL!
  }
  darray_clear(updateQueue);

  // Handle destruction of meshes.
  darray* destroyQueue = trigon_instance.meshManager.destroyCommandQueues + frame;
  darray_iterator destroyQueueIt = darray_iterate(destroyQueue);
  while(darray_iterator_has_next(&destroyQueueIt)){
    mesh_destroy_command* command = darray_iterator_next(&destroyQueueIt);
    // BACKEND CALL!
  }
  darray_clear(destroyQueue);

  return 0;
}
