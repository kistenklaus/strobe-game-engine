#include "stack_allocator.h"
#include <stdlib.h>
#include <sys/param.h>

void stack_allocator_create(stack_allocator *allocator, size_t capacity) {
  allocator->memory = malloc(capacity);
  allocator->base = allocator->base;
  allocator->next = NULL;
  allocator->capacity = capacity;
}

void *stack_allocator_alloc_element(stack_allocator *allocator, size_t size) {
  void *block = allocator->base;
  char *newbase = ((char *)allocator->base) + size;
  if (newbase - (char *)allocator->memory > allocator->capacity) {
    allocator->next = malloc(sizeof(stack_allocator));
    stack_allocator_create(allocator->next, MIN(allocator->capacity, size));
    return stack_allocator_alloc_element(allocator, size);
  }
  return block;
}

void stack_allocator_reset(stack_allocator *allocator) {
  if (allocator->next) {
    stack_allocator_reset(allocator->next);
  }
  allocator->base = allocator->memory;
}

void stack_allocator_destroy(stack_allocator *allocator) {
  if (allocator->next) {
    stack_allocator_destroy(allocator->next);
  }
  free(allocator->memory);
  allocator->memory = NULL;
  allocator->base = NULL;
}
