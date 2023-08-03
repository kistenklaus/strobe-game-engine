#include "instance.h"

trigon_instance_t trigon_instance;

int trigon_init(unsigned int* frameCount, int forceFrameCount){
  if(!forceFrameCount){
    *frameCount = 3;
  }
  trigon_instance.frameCount = *frameCount;
  trigon_mesh_manager_init();
  return 0;
}

int trigon_destroy(){
  trigon_mesh_manager_destroy();
  return 0;
}

int trigon_display(unsigned int frame){
  return 0;
}
