#include "darray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_int(const void *va, const void *vb)
{
  int a = *(int *)va, b = *(int *) vb;
  return a < b ? -1 : a > b ? +1 : 0;
}

static void darray_grow(darray *darray) {
  void *oldMemory = darray->memory;
  size_t newCap = darray->capacity * 3 / 2 + 1;
  void *newMemory = calloc(newCap, darray->elementSize);
  memcpy(newMemory, darray->memory, darray->capacity * darray->elementSize);
  darray->memory = newMemory;
  darray->capacity = newCap;
  free(oldMemory);
}

void darray_create(darray *darray, unsigned int elementSize,
                   unsigned int capacity) {
  darray->elementSize = elementSize;
  darray->capacity = capacity;
  darray->memory = calloc(capacity, elementSize);
  darray->count = 0;
}

void *darray_emplace_back(darray *darray) {
  if (darray->count >= darray->capacity - 1) {
    darray_grow(darray);
  }
  void *block = darray_get(darray, darray->count);
  darray->count++;
  return block;
}

void darray_push_back(darray *darray, void *element) {
  if (darray->count >= darray->capacity - 1) {
    darray_grow(darray);
  }

  memcpy(darray_get(darray, darray->count), element, darray->elementSize);
  darray->count++;
}

void darray_pop_back(darray *darray) { darray->count--; }

void *darray_get_back(darray *darray) {
  return darray_get(darray, darray->count - 1);
}

void darray_set(darray *darray, size_t i, void *data) {
  memcpy(darray_get(darray, i), data, darray->elementSize);
}

void *darray_get(darray *darray, size_t i) {
  return ((char *)darray->memory) + (i)*darray->elementSize;
}

void darray_clear(darray *darray) { darray->count = 0; }

size_t darray_find(darray *darray, int (*cmp)(void *elem)) {
  size_t i = 0;
  void *cur = darray->memory;
  for (; i < darray->count; i++, cur = (((char *)cur) + darray->elementSize)) {
    if (cmp(cur)) {
      return i;
    }
  }
  return i;
}

void darray_sort(darray* darray, int (*cmp)(const void* a, const void* b)){
  qsort(darray->memory, darray->count, darray->elementSize, cmp);
}

void darray_destroy(darray *darray) {
  free(darray->memory);
  darray->elementSize = 0;
  darray->capacity = 0;
}

int darray_is_empty(darray *darray) { return darray->count; }

size_t darray_size(darray *darray) { return darray->count; }

darray_iterator darray_iterate(darray *darray) {
  darray_iterator iterator;
  iterator.cur = darray->memory;
  iterator.end =
      ((char *)darray->memory) + darray->count * darray->elementSize;
  iterator.elementSize = darray->elementSize;
  return iterator;
}

void *darray_iterator_next(darray_iterator *iterator) {
  void *it = iterator->cur;
  iterator->cur = ((char *)iterator->cur) + iterator->elementSize;
  return it;
}
int darray_iterator_has_next(darray_iterator *iterator) {
  return iterator->cur != iterator->end; 
}
