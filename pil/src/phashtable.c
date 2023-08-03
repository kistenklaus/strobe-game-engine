#include "phashtable.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

uint32_t MurmurOAAT_32(const char* str, uint32_t h)
{
    // One-byte-at-a-time hash based on Murmur's mix
    // Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

int phashtable_create(phashtable_t* hashtable, const char** keys, unsigned int count){
  hashtable->seed = 0x1498276;
  hashtable->keys = calloc(count, sizeof(const char*));
  float factor = pow(count / 40.0f,1.5f) + 1;
  hashtable->size = count * factor;
  hashtable->redirectionTable = calloc(hashtable->size, sizeof(unsigned int));
  for(unsigned int i = 0; i < count; i++){
    size_t keylen = strlen(keys[i]);
    hashtable->keys[i] = calloc(keylen + 1, sizeof(char));
    memcpy(hashtable->keys[i], keys[i], keylen);
  }
  
  int* collisionTable = calloc(hashtable->size, sizeof(int));

  unsigned int iterations = 0;
  goto skip;
reset:
  for(unsigned int i = 0; i < hashtable->size; i++){
    collisionTable[i] = 0;
  }
skip:
  iterations++;
  if(iterations > 1000000){
    free(collisionTable);
    phashtable_destroy(hashtable);
    return -1;
  }
  for(unsigned int i = 0; i < count; i++){
    uint32_t hash = MurmurOAAT_32(hashtable->keys[i], hashtable->seed);
    unsigned int index = hash % hashtable->size;
    if(collisionTable[index]){
      //collision.
      hashtable->seed += 1; // +1 to avoid any chance of cycles.
      goto reset;
    }
    hashtable->redirectionTable[index] = i;
    collisionTable[index] = 1;
  }

  free(collisionTable);

  return 0;
}

int phashtable_get(phashtable_t* hashtable, const char* key, unsigned int* value){
  uint32_t hash = MurmurOAAT_32(key, hashtable->seed);
  *value = hashtable->redirectionTable[hash % hashtable->size];
  return !strcmp(hashtable->keys[*value], key);
}

int phashtable_destroy(phashtable_t* hashtable){
  if(hashtable == NULL)return 1;
  if(hashtable->keys != NULL){
    for(unsigned int i = 0; i < hashtable->size; i++){
      free(hashtable->keys[i]);
    }
  }
  free(hashtable->keys);
  free(hashtable->redirectionTable);
  return 0;
}
