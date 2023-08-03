#ifndef STROBE_RENDERER_BACKEND_TRIGON_INSTANCE_H 
#define STROBE_RENDERER_BACKEND_TRIGON_INSTANCE_H 

#include "mesh_manager.h"
#include "material_manager.h"

typedef struct {
  unsigned int frameCount;
  trigon_mesh_manager meshManager;
  trigon_material_manager materialManager;
} trigon_instance_t;

extern trigon_instance_t trigon_instance;

int trigon_init(unsigned int* frameCount, int forceFrameCount);

int trigon_destroy();

int trigon_display(unsigned int frame);

#endif
