#ifndef STROBE_RENDERER_MATERIAL_H
#define STROBE_RENDERER_MATERIAL_H

#include "render_backend.h"
#include "render_context.h"

int renderer_get_avaiable_materials(render_context_t *context,
                                    material_description **materialDescriptions,
                                    unsigned int *materialCount);

int renderer_create_material(render_context_t *context, unsigned int *material,
                             material_create_info *createInfo);

int renderer_get_material_properties(
    render_context_t *context, material_description *materialDescription,
    material_property_description **properties);

int renderer_get_material_property(render_context_t *context,
                                   unsigned int material,
                                   material_property_description *property,
                                   void *out);

int renderer_set_material_property(render_context_t *context,
                                   unsigned int material,
                                   material_property_description *property,
                                   void *value);

int renderer_destroy_material(render_context_t *context, unsigned int material);

// utility.
int renderer_get_material_description_by_name(
    render_context_t *context, material_description *materialDescription,
    const char *materialName);

int renderer_get_material_named_property(render_context_t* context, 
    unsigned int material, const char* propertyName, void* value);

int renderer_set_material_named_property(render_context_t* context, 
    unsigned int material, const char* propertyName, void* value);



#endif
