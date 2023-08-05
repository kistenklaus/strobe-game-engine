#ifndef STROBE_RENDERER_COMMON_API_H
#define STROBE_RENDERER_COMMON_API_H
#include "shared_memory.h"
#include "mat.h"

typedef enum {
  STROBE_MESH_ATTRIBUTE_USAGE_STATIC,
  STROBE_MESH_ATTRIBUTE_USAGE_DYNAMIC
} mesh_attribute_usage;

typedef struct {
  shared_memory data;
  unsigned int stride;
  unsigned int offset;
  unsigned int size;
  mesh_attribute_usage usage;
} mesh_attribute_create_info;

extern const mesh_attribute_create_info MESH_NO_ATTIBUTE_CREATE_INFO;

typedef struct {
  mesh_attribute_create_info positions;
  mesh_attribute_create_info textureCoords;
  mesh_attribute_create_info normals;
  mesh_attribute_create_info tangents;
  mesh_attribute_create_info bitangents;
  mesh_attribute_create_info colors;
  unsigned int vertexCount;
} mesh_create_info;

typedef enum {
  MESH_ATTRIBUTE_POSITION,
  MESH_ATTRIBUTE_TEXTURE_COORDS,
  MESH_ATTRIBUTE_NORMALS,
  MESH_ATTRIBUTE_TANGENTS,
  MESH_ATTRIBUTE_BITANGENTS
} mesh_attribute_type;

typedef struct {

} mesh_update_info;

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

} render_backend;


typedef struct {
  int (*create_backend)(render_backend* backend);
} render_backend_create_info;


#endif
