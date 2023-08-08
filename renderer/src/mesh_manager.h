#ifndef STROBE_RENDERER_MESH_MANAGER_H
#define STROBE_RENDERER_MESH_MANAGER_H

#include "threading.h"
#include "allocators.h"
#include "containers.h"
#include "render_backend.h"
#include "mesh_def.h"


typedef struct {
  unsigned int frameCount;
} mesh_manager_create_info;

typedef struct {
  pool_allocator allocator;
  darray idLookup; //of unsigned int
  mutex idLookupMutex;
  unsigned int frameCount;
  darray* createCommandQueues; //of 
  darray* destroyCommandQueues;
  darray* updateCommandQueues; //
}mesh_manager;

int renderer_mesh_manager_init(mesh_manager* manager, mesh_manager_create_info* createInfo);

int renderer_mesh_manager_create_mesh(mesh_manager* manager, unsigned int frame, unsigned int* mesh, mesh_create_info* meshCreateInfo);

int renderer_mesh_manager_destroy_mesh(mesh_manager* manager, unsigned int frame, unsigned int mesh);

int renderer_mesh_manager_update_mesh(mesh_manager* manager, unsigned int frame, unsigned int mesh, mesh_update_info* updateInfo);

int renderer_mesh_manager_prepare_frame(mesh_manager* manager, unsigned int frame);

int renderer_mesh_manager_destroy(mesh_manager* manager);

#endif



