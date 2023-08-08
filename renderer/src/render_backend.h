#ifndef STROBE_RENDERER_RENDER_BACKEND_H
#define STROBE_RENDERER_RENDER_BACKEND_H

#include "material_def.h"
#include "mesh_def.h"
#include "window.h"

typedef struct render_backend {
  int (*init)(struct render_backend *backend, unsigned int* frameCount, window_t* window);
  void (*destroy)(struct render_backend *backend);
  void (*beginFrame)(struct render_backend *backend);
  void (*endFrame)(struct render_backend *backend);

  void (*create_mesh)(struct render_backend *backend, unsigned int *mesh,
                      mesh_create_info *createInfo);
  void (*destroy_mesh)(struct render_backend *backend, unsigned int mesh);
  void (*bind_mesh)(struct render_backend *backend, unsigned int mesh);
  void (*update_mesh)(struct render_backend *backend, unsigned int mesh,
                      mesh_update_info *updateInfo);

  void (*create_material)(struct render_backend *backend,
                          unsigned int *material,
                          material_create_info *createInfo);
  void (*destroy_material)(struct render_backend *backend,
                           unsigned int material);
  void (*bind_material)(struct render_backend *backend, unsigned int material);
  void (*update_material)(struct render_backend *backend, unsigned int material,
                          material_update_info *updateInfo);

  void (*get_avaiable_materials)(struct render_backend *backend,
                                 material_description *materials);
  unsigned int (*get_avaiable_material_count)(struct render_backend *backend);
  void (*get_material_properties)(struct render_backend *backend,
                                  material_description *mat,
                                  material_property_description *properties);
  unsigned int (*get_material_property_count)(struct render_backend* backend, 
      material_description* mat);
} render_backend;

typedef struct {
  void (*constructor)(render_backend *backend);
} render_backend_create_info;

#endif
