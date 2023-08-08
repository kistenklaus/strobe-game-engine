#include "opengl_backend.h"
#include "log.h"
#include "material_def.h"
#include "window.h"

static int opengl_backend_init(render_backend *backend,
                                unsigned int *frameCount, window_t *window) {
  logInfo("[OpenGL-backend     ] initalize\n");
  *frameCount = 3;
  return 0;
}

static void opengl_backend_destroy(render_backend *backend) {
  logInfo("[OpenGL-backend     ] destroy\n");
}

static void opengl_backend_begin_frame(render_backend *backend) {
  logTrace("[OpenGL-backend    ] begin frame\n");
}

static void opengl_backend_end_frame(render_backend *backend) {
  logTrace("[OpenGL-backend    ] end frame\n");
}

static void opengl_backend_create_mesh(render_backend *backend,
                                       unsigned int *mesh,
                                       mesh_create_info *createInfo) {
  logDebug("[OpenGL-backend    ] create mesh\n");
}

static void opengl_backend_destroy_mesh(render_backend *backend,
                                        unsigned int mesh) {
  logDebug("[OpenGL-backend    ] destroy mesh\n");
}

static void opengl_backend_bind_mesh(render_backend *backend,
                                     unsigned int mesh) {
  logTrace("[OpenGL-backend    ] bind mesh\n");
}

static void opengl_backend_update_mesh(render_backend *backend,
                                       unsigned int mesh,
                                       mesh_update_info *updateInfo) {
  logTrace("[OpenGL-backend    ] update mesh\n");
}

static void opengl_backend_create_material(render_backend *backend,
                                           unsigned int *material,
                                           material_create_info *createInfo) {
  logDebug("[OpenGL-backend    ] create material\n");
}

static void opengl_backend_destroy_material(render_backend *backend,
                                            unsigned int material) {
  logDebug("[OpenGL-backend    ] destroy material\n");
}

static void opengl_backend_bind_material(render_backend *backend,
                                         unsigned int material) {
  logTrace("[OpenGL-backend    ] bind material\n");
}

static void opengl_backend_update_material(render_backend *backend,
                                           unsigned int material,
                                           material_update_info *updateInfo) {
  logTrace("[OpenGL-backend    ] update material\n");
}

static void
opengl_backend_get_avaiable_materials(render_backend *backend,
                                      material_description *materials) {
  logInfo("[OpenGL-backend    ] fetch avaiable materials\n");
}

static unsigned int
opengl_backend_get_avaiable_material_count(render_backend *backend) {
  logInfo("[OpenGL-backend    ] fetch avaiable material count\n");
  return 0;
}

static void opengl_backend_get_material_properties(
    render_backend *backend, material_description *material,
    material_property_description *properties) {
  logInfo("[OpenGL-backend    ] fetch material properties\n");
}

static unsigned int
opengl_backend_get_material_property_count(render_backend *backend,
                                           material_description *material) {
  logInfo("[OpenGL-backend    ] fetch material property count\n");
  return 0;
}

static void opengl_backend_link(render_backend *backend) {
  backend->init = opengl_backend_init;
  backend->destroy = opengl_backend_destroy;
  backend->beginFrame = opengl_backend_begin_frame;
  backend->endFrame = opengl_backend_end_frame;
  backend->create_mesh = opengl_backend_create_mesh;
  backend->destroy_mesh = opengl_backend_destroy_mesh;
  backend->bind_mesh = opengl_backend_bind_mesh;
  backend->update_mesh = opengl_backend_update_mesh;
  backend->create_material = opengl_backend_create_material;
  backend->destroy_material = opengl_backend_destroy_material;
  backend->bind_material = opengl_backend_bind_material;
  backend->update_material = opengl_backend_update_material;
  backend->get_avaiable_materials = opengl_backend_get_avaiable_materials;
  backend->get_avaiable_material_count =
      opengl_backend_get_avaiable_material_count;
  backend->get_material_properties = opengl_backend_get_material_properties;
  backend->get_material_property_count =
      opengl_backend_get_material_property_count;
}

render_backend_create_info OPENGL_RENDER_BACKEND = {.constructor =
                                                        opengl_backend_link};
