#ifndef STROBE_PIL_POOL_ALLOCATOR_H
#define STROBE_PIL_POOL_ALLOCATOR_H

#include <stddef.h>
typedef struct pool_allocator_chunk{
  void* memory;
  struct pool_allocator_chunk* nextChunk;
} pool_allocator_chunk;

typedef struct pool_allocator_element {
  struct pool_allocator_element* next;
} pool_allocator_element;

typedef struct {
  pool_allocator_chunk chunkHead;
  pool_allocator_element* elementHead;
  size_t elementSize;
  size_t chunkSize;
} pool_allocator;


void pool_allocator_create(pool_allocator* poolAllocator, size_t elementSize, size_t chunkSize);

void* pool_allocator_alloc_element(pool_allocator* poolAllocator);

void pool_allocator_free_element(pool_allocator* poolAllocator, void* element);

void pool_allocator_destroy(pool_allocator* poolAllocator);


#endif
