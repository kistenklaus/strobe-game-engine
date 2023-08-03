#include "material_manager.h"
#include <stdio.h>
#include "instance.h"

int trigon_get_materials(material_description* materialDescriptions){
  materialDescriptions[0].id = 0;
  materialDescriptions[0].name = "just-color";
  materialDescriptions[0].propertyCount = 1;

  return 0;
}

int trigon_get_material_count(unsigned int* materialCount){
  *materialCount = 1;
  return 0;
}

int trigon_create_material(unsigned int frame, unsigned int* material, material_create_info* materialCreateInfo){
  return 0;
}

int trigon_get_material_description(unsigned int material, material_description* materialDescription){
  return 0;
}

int trigon_get_material_properties(material_description* materialDescription, material_property_description* properties){
  switch(materialDescription->id){
    case 0:
      properties[0].name = "color";
      properties[0].size = 4 * sizeof(float);
      properties[0].typeHint = "vec4";
      return 0;
  }
  return -1;
}

int trigon_set_material_property(unsigned int frame, unsigned int material, material_property_description* property, void* data){
  return 0;
}

int trigon_get_material_property(unsigned int frame, unsigned int material, material_property_description* property, void* data){
  return 0;
}

int trigon_destroy_material(unsigned int frame, unsigned int material){
  return 0;
}

