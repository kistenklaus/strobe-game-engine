#ifndef STROBE_PIL_D_ARRAY_H
#define STROBE_PIL_D_ARRAY_H

#include <stddef.h>
typedef struct {
  void* memory;
  unsigned int elementSize;
  unsigned int capacity;
  unsigned int count;
} darray;

void darray_create(darray* darray, unsigned int elementSize, unsigned int capacity);

void* darray_emplace_back(darray* darray);

void darray_push_back(darray* darray, void* element);

void darray_pop_back(darray* darray);

void* darray_get_back(darray* darray);

void darray_set(darray* darray, size_t i, void* data);

void* darray_get(darray* darray, size_t i);

void darray_clear(darray* darray);

size_t darray_find(darray* darray, int (*cmp)(void* elem));

int darray_is_empty(darray* darray);

size_t darray_size(darray* darray);

void darray_destroy(darray* darray);

typedef struct {
  void* end;
  void* cur;
  unsigned int elementSize;
} darray_iterator;

darray_iterator darray_iterate(darray* darray);

void* darray_iterator_next(darray_iterator* iterator);
int darray_iterator_has_next(darray_iterator* iterator);

#endif
