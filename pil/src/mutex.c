#include "mutex.h"
#include <pthread.h>


int mutex_init(mutex* mutex){
  return pthread_mutex_init(&mutex->pmutex_t, NULL);
}

int mutex_lock(mutex* mutex){
  return pthread_mutex_lock(&mutex->pmutex_t);
}

int mutex_trylock(mutex* mutex){
  return pthread_mutex_trylock(&mutex->pmutex_t);
}

int mutex_unlock(mutex* mutex){
  return pthread_mutex_unlock(&mutex->pmutex_t);
}

int mutex_destroy(mutex* mutex){
  return pthread_mutex_destroy(&mutex->pmutex_t);
}
