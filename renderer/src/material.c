#include "material.h"
#include "phashtable.h"
#include "render_backend.h"
#include <stdlib.h>
#include <string.h>

static void require_material_count(render_context_t *context) {
  if (context->materialCount == 0) {
    context->backend.get_material_count(&context->materialCount);
  }
}

static void require_material_descriptions(render_context_t *context) {
  if (context->materialDescriptions == NULL) {
    require_material_count(context);
    context->materialDescriptions =
        calloc(context->materialCount, sizeof(material_description));
    context->backend.get_materials(context->materialDescriptions);
  }
}

static void
require_material_property_description_buffer(render_context_t *context) {
  if (context->materialPropertyDescriptions == NULL) {
    require_material_count(context);
    context->materialPropertyDescriptions =
        calloc(context->materialCount, sizeof(material_property_description *));
  }
}

static void require_material_property_descriptions(
    render_context_t *context, material_description *materialDescription) {
  require_material_property_description_buffer(context);
  if (context->materialPropertyDescriptions[materialDescription->id] == NULL) {
    context->materialPropertyDescriptions[materialDescription->id] =
        calloc(materialDescription->propertyCount,
               sizeof(material_property_description));
    context->backend.get_material_properties(
        materialDescription,
        context->materialPropertyDescriptions[materialDescription->id]);
  }
}

int renderer_get_avaiable_materials(render_context_t *context,
                                    material_description **materialDescriptions,
                                    unsigned int *materialCount) {
  require_material_descriptions(context);
  *materialDescriptions = context->materialDescriptions;
  *materialCount = context->materialCount;
  return 0;
}

int renderer_create_material(render_context_t *context, unsigned int *material,
                             material_create_info *createInfo) {
  return context->backend.create_material(context->submitFrame, material, createInfo);
}

int renderer_get_material_properties(
    render_context_t *context, material_description *materialDescription,
    material_property_description **properties) {
  require_material_property_descriptions(context, materialDescription);
  *properties = context->materialPropertyDescriptions[materialDescription->id];
  return 0;
}

int renderer_get_material_property(render_context_t *context,
                                   unsigned int material,
                                   material_property_description *property,
                                   void *out) {
  return context->backend.get_material_property(context->submitFrame, material, property, out);
}

int renderer_set_material_property(render_context_t *context,
                                   unsigned int material,
                                   material_property_description *property,
                                   void *value) {
  return context->backend.set_material_property(context->submitFrame, material, property, value);
}

int renderer_destroy_material(render_context_t *context,
                              unsigned int material) {
  return context->backend.destroy_material(context->submitFrame, material);
}


int renderer_get_material_description_by_name(
    render_context_t *context, material_description *materialDescription,
    const char *materialName){
  require_material_descriptions(context);
  for(unsigned int i = 0; i < context->materialCount; i++) {
    if(!strcmp(materialName, context->materialDescriptions[i].name)){
      *materialDescription = context->materialDescriptions[i];
      return 0;
    }
  }
  return -1;
}

static int linear_search_properties(render_context_t* context, material_description* materialDesc, 
    const char* propertyName, material_property_description** propertyDescription){
  for(unsigned int i = 0; i < materialDesc->propertyCount; i++){
    if(!strcmp(propertyName, context->materialPropertyDescriptions[materialDesc->id]->name)){
      return 0;
    }
  }
  return -1;
}

int renderer_get_material_named_property(render_context_t* context, 
    unsigned int material, const char* propertyName, void* out){
  material_description materialDesc;
  context->backend.get_material_description(material, &materialDesc);
  material_property_description* propertyDesc;
  if(!linear_search_properties(context, &materialDesc, propertyName, &propertyDesc)){
    context->backend.get_material_property(context->submitFrame, material, propertyDesc, out);
    return 0;
  }
  return -1;
}

int renderer_set_material_named_property(render_context_t* context, 
    unsigned int material, const char* propertyName, void* value){
  material_description materialDesc;
  context->backend.get_material_description(material, &materialDesc);
  material_property_description* propertyDesc;
  if(!linear_search_properties(context, &materialDesc, propertyName, &propertyDesc)){
    context->backend.get_material_property(context->submitFrame, material, propertyDesc, value);
    return 0;
  }
  return -1;
}
