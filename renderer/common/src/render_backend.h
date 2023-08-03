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
} mesh_create_info;

typedef struct {
  void* data;
  unsigned int size;
  unsigned int stride;
  unsigned int offset;
  unsigned int meshId;
} mesh_mapped_attribute;

typedef enum {
  MESH_ATTRIBUTE_POSITION,
  MESH_ATTRIBUTE_TEXTURE_COORDS,
  MESH_ATTRIBUTE_NORMALS,
  MESH_ATTRIBUTE_TANGENTS,
  MESH_ATTRIBUTE_BITANGENTS
} mesh_attribute_type;

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
  int (*init)(unsigned int* frameCount, int forceFrameCount);
  int (*destroy)();
  int (*display)(unsigned int frame);

  int (*create_mesh)(unsigned int frame, unsigned int * mesh, mesh_create_info* meshCreateInfo);
  int (*destroy_mesh)(unsigned int frame, unsigned int mesh);
  int (*map_mesh_attribute)(unsigned int frame, unsigned int mesh, mesh_attribute_type attributeType, mesh_mapped_attribute* attribute);
  int (*unmap_mesh_attribute)(unsigned int frame, mesh_mapped_attribute* attribute);

  int (*get_material_count)(unsigned int* materialCount);
  int (*get_materials)(material_description* materialDescriptions);
  int (*create_material)(unsigned int frame, unsigned int* material, material_create_info* materialDescription);
  int (*get_material_description)(unsigned int material, material_description* materialDescription);
  int (*get_material_properties)(material_description* materialDescription, material_property_description* properties);
  int (*set_material_property)(unsigned int frame, unsigned int material, material_property_description* property, void* data);
  int (*get_material_property)(unsigned int frame, unsigned int material, material_property_description* property, void* data);
  int (*destroy_material)(unsigned int frame, unsigned int material);

  int (*draw)(unsigned int frame, unsigned int mesh, unsigned int material, mat4f* transform);

} render_backend;


typedef struct {
  int (*create_backend)(render_backend* backend);
} render_backend_create_info;


#endif
