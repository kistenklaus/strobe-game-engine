#ifndef STROBE_RENDERER_PIL_STACK_ALLOCATOR_H
#define STROBE_RENDERER_PIL_STACK_ALLOCATOR_H

#include <stddef.h>
typedef struct stack_allocator{
  void* memory;
  void* base;
  struct stack_allocator* next;
  size_t capacity;
} stack_allocator;

void stack_allocator_create(stack_allocator* allocator, size_t capacity);

void* stack_allocator_alloc_element(stack_allocator* allocator, size_t size);

void stack_allocator_reset(stack_allocator* allocator);

void stack_allocator_destroy(stack_allocator* allocator);



#endif
