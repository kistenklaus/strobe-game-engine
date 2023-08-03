#include "pool_allocator.h"
#include "assert.h"
#include <stdlib.h>

static void pool_allocator_chunk_create(pool_allocator_chunk* chunk, size_t elementSize, 
    size_t chunkSize) {
  chunk->nextChunk = NULL;
  chunk->memory = calloc(chunkSize, elementSize);
  // Setup freelist.
  for (size_t i = 0; i < chunkSize - 1; i++) {
    ((pool_allocator_element *)((char *)chunk->memory +
                                i * elementSize))
        ->next =
        (pool_allocator_element *)(((char *)chunk->memory) +
                                   i * elementSize);
  }

  ((pool_allocator_element *)((char *)chunk->memory +
                              (chunkSize - 1) * elementSize))->next = NULL;
}

void pool_allocator_create(pool_allocator *poolAllocator, size_t elementSize,
                           size_t chunkSize) {
  assert(chunkSize > 0);
  if (elementSize < sizeof(pool_allocator_element)) {
    elementSize = sizeof(pool_allocator_element);
  }
  poolAllocator->chunkSize = chunkSize;
  poolAllocator->elementSize = elementSize;
  pool_allocator_chunk_create(&poolAllocator->chunkHead, elementSize, chunkSize);
  
  poolAllocator->elementHead = poolAllocator->chunkHead.memory;
}

void *pool_allocator_alloc_element(pool_allocator *poolAllocator) {
  // Check if freelist is empty.
  if( poolAllocator->elementHead == NULL){
    // Allocate new chunk.
    pool_allocator_chunk* tailChunk = &poolAllocator->chunkHead;
    while(tailChunk->nextChunk){
      tailChunk = tailChunk->nextChunk;
    }
    tailChunk->nextChunk = malloc(sizeof(pool_allocator_chunk));
    pool_allocator_chunk_create(tailChunk->nextChunk, poolAllocator->elementSize, poolAllocator->chunkSize);
    poolAllocator->elementHead = tailChunk->memory;
  }
  void* element = poolAllocator->elementHead;
  poolAllocator->elementHead = poolAllocator->elementHead->next;
  return element;
}

void pool_allocator_free_element(pool_allocator *poolAllocator, void *element) {
  pool_allocator_element* block = element;
  block->next = poolAllocator->elementHead;
  poolAllocator->elementHead = block;
}

static void pool_allocator_chunk_destroy(pool_allocator_chunk* chunk) {
  if(chunk->nextChunk){
    pool_allocator_chunk_destroy(chunk->nextChunk);
    chunk->nextChunk = NULL;
  }
  free(chunk->memory);
  chunk->memory = NULL;
}

void pool_allocator_destroy(pool_allocator *poolAllocator) {
  pool_allocator_chunk_destroy(&poolAllocator->chunkHead);
  poolAllocator->elementHead = NULL;
}
