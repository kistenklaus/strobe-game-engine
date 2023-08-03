#ifndef STROBE_RENDERER_BACKEND_TRIGON_MESH_MANAGER_H
#define STROBE_RENDERER_BACKEND_TRIGON_MESH_MANAGER_H

#include "render_backend.h"
#include "pool_allocator.h"
#include "darray.h"
#include "stack_allocator.h"
#include "shared_memory.h"

typedef struct {
} mesh_control_block;

typedef struct {
  pool_allocator allocator;
  darray idLookup; //of unsigned int
  stack_allocator* argumentAllocators;
  darray* createCommandQueues; //of 
  darray* destroyCommandQueues;
  darray* updateCommandQueues; //
  darray* updateLookup; //
} trigon_mesh_manager;

typedef struct {
  mesh_create_info createInfo;
  mesh_control_block* controlBlock;
} mesh_create_command;

typedef struct {
  mesh_control_block* controlBlock;
} mesh_destroy_command;

typedef struct {
  unsigned int startVertex;
  unsigned int count;
  shared_memory data; // vertex attribute data.
  mesh_attribute_type type;
} mesh_attribute_update_info;

typedef struct {
  shared_memory attributeUpdateInfos; // array of mesh_attribute_update_infos
  unsigned int attributeUpdateInfoCount;
} mesh_update_info;


typedef struct {
  mesh_control_block* controlBlock;
  mesh_update_info updateInfo;
} mesh_update_command;


int trigon_mesh_manager_init();

int trigon_create_mesh(unsigned int frame, unsigned int *mesh, mesh_create_info* meshCreateInfo);

int trigon_destroy_mesh(unsigned int frame, unsigned int mesh);

int trigon_update_mesh(unsigned int frame, unsigned int mesh, mesh_update_info* updateInfo);

int trigon_mesh_manager_destroy();

int trigon_mesh_manager_prepare_frame(unsigned int frame);

#endif
