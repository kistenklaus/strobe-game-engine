#ifndef STROBE_PIL_PHASHTABLE_H
#define STROBE_PIL_PHASHTABLE_H

typedef struct {
  unsigned long seed;
  char** keys;
  unsigned int* redirectionTable;
  unsigned int size;
} phashtable_t;

int phashtable_create(phashtable_t* hashtable, const char** keys, unsigned int count);

int phashtable_get(phashtable_t* hashtable, const char* key, unsigned int* value);

int phashtable_destroy(phashtable_t* hashtable);

#endif
