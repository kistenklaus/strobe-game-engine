#ifndef STROBE_PIL_SHARED_MEMORY_H
#define STROBE_PIL_SHARED_MEMORY_H
#include <stddef.h>

typedef struct {
  void *data;
} shared_memory;

void shared_memory_create(shared_memory *sharedMemory, size_t size);

void shared_memory_make_shared(shared_memory sharedMemory);

void *shared_memory_get(shared_memory sharedMemory);

int shared_memory_is_null(shared_memory sharedMemory);

void shared_memory_free(shared_memory sharedMemory);

#endif
