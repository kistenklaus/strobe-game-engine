#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>

void shared_memory_create(shared_memory* sharedMemory, size_t size) {
  sharedMemory->data = malloc(size + 2 * sizeof(size_t));
  ((size_t *)(sharedMemory->data))[0] = 1;
  ((size_t *)(sharedMemory->data))[1] = 1; //actually no idea why we allocate 2 additional bytes.
}

void shared_memory_make_shared(shared_memory sharedMemory) {
  (*((size_t *)(sharedMemory.data)))++;
}

void* shared_memory_get(shared_memory sharedMemory) {
  return ((size_t *)(sharedMemory.data)) + 2;
}


int shared_memory_is_null(shared_memory sharedMemory) {
  return sharedMemory.data == NULL;
}

void shared_memory_free(shared_memory sharedMemory) {
  (*(size_t *)(sharedMemory.data))--;
  if (*(size_t *)(sharedMemory.data) == 0) {
    free(sharedMemory.data);
  }
}
