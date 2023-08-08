#ifndef STROBE_RENDERER_MATERIAL_MANAGER_H
#define STROBE_RENDERER_MATERIAL_MANAGER_H

#include "threading.h"
#include "allocators.h"
#include "containers.h"
#include "render_backend.h"
#include "material_def.h"


typedef struct {
  unsigned int frameCount;
} material_manager_create_info;

typedef struct {
  pool_allocator allocator;
  darray idLookup; //of unsigned int
  mutex idLookupMutex;
  unsigned int frameCount;
  darray* createCommandQueues; //of 
  darray* destroyCommandQueues;
  darray* updateCommandQueues; //
} material_manager;

int renderer_material_manager_init(material_manager* manager, material_manager_create_info* createInfo);

int renderer_material_manager_create_material(material_manager* manager, unsigned int frame, unsigned int* material, material_create_info* createInfo);

int renderer_material_manager_destroy_material(material_manager* manager, unsigned int frame, unsigned int material);

int renderer_material_manager_prepare_frame(material_manager* manager, unsigned int frame);

int renderer_material_manager_destroy(material_manager* manager);

#endif
