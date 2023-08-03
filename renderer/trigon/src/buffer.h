#ifndef STROBE_RENDERER_BACKEND_TRIGON_BUFFER_H
#define STROBE_RENDERER_BACKEND_TRIGON_BUFFER_H

#include <stddef.h>
typedef enum {
  TRIGON_BUFFER_TYPE_ARRAY_BUFFER,
  TRIGON_BUFFER_TYPE_ELEMENT_BUFFER,
  TRIGON_BUFFER_TYPE_VERTEX_ARRAY_BUFFER
} trigon_buffer_type;

typedef enum {
  TRIGON_BUFFER_USAGE_STATIC,
  TRIGON_BUFFER_USAGE_DYNAMIC
} trigon_buffer_usage;

typedef struct {
  trigon_buffer_type type;
  trigon_buffer_usage usage;
  size_t size;
} trigon_buffer_create_info;

typedef struct {
  
} trigon_buffer;

void trigon_buffer_create(trigon_buffer* buffer, trigon_buffer_create_info* createInfo);

void trigon_buffer_destroy(trigon_buffer* buffer);

#endif
