#include "material_manager.h"
#include "render_backend.h"
#include "instance.h"
#include "mesh_manager.h"
#include "submission.h"

static int link(render_backend* backend){
  backend->init = trigon_init;
  backend->destroy = trigon_destroy;
  backend->display = trigon_display;

  backend->create_mesh = trigon_create_mesh;
  backend->destroy_mesh = trigon_destroy_mesh;
  
  backend->get_material_count = trigon_get_material_count;
  backend->get_materials = trigon_get_materials;
  backend->create_material = trigon_create_material;
  backend->destroy_material = trigon_destroy_material;
  backend->get_material_description = trigon_get_material_description;
  backend->get_material_properties = trigon_get_material_properties;
  backend->get_material_property = trigon_get_material_property;
  backend->set_material_property = trigon_set_material_property;

  backend->draw = trigon_draw;

  return 0;
}

const render_backend_create_info TRIGON_BACKEND = {
  .create_backend = link
};
