#ifndef STROBE_RENDERER_MATERIAL_DEF_H
#define STROBE_RENDERER_MATERIAL_DEF_H

typedef struct {
  const char* name;
  const char* typeHint;
  unsigned int size;
} material_property_description;

typedef struct {
  const char* name;
  unsigned int id;
  unsigned int propertyCount;
} material_description;

typedef struct {
  material_description description;
  void* initalPropertyValues;
} material_create_info;

typedef struct {

} material_update_info;

#endif
