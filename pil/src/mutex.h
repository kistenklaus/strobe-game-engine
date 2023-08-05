#ifndef STROBE_PIL_MUTEX_H
#define STROBE_PIL_MUTEX_H

#include "pthread.h"

typedef struct {
  pthread_mutex_t pmutex_t;
} mutex;

int mutex_init(mutex* mutex);

int mutex_lock(mutex* mutex);

int mutex_trylock(mutex* mutex);

int mutex_unlock(mutex* mutex);

int mutex_destroy(mutex* mutex);

#endif
