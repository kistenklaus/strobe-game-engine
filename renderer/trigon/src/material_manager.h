#ifndef STROBE_RENDERER_BACKEND_TRIGON_MATERIAL_MANAGER_H
#define STROBE_RENDERER_BACKEND_TRIGON_MATERIAL_MANAGER_H


#include "render_backend.h"

typedef struct {

} trigon_material_manager;

int trigon_get_materials(material_description* materialDescriptions);

int trigon_get_material_count(unsigned int* materialCount);

int trigon_create_material(unsigned int frame, unsigned int* material, material_create_info* materialCreateInfo);

int trigon_get_material_description(unsigned int material, material_description* materialDescription);

int trigon_get_material_properties(material_description* materialDescription, material_property_description* properties);

int trigon_set_material_property(unsigned int frame, unsigned int material, material_property_description* property, void* data);

int trigon_get_material_property(unsigned int frame, unsigned int material, material_property_description* property, void* data);

int trigon_destroy_material(unsigned int frame, unsigned int material);





#endif
