#ifndef STROBE_RENDERER_PIL_THREAD_H
#define STROBE_RENDERER_PIL_THREAD_H

#include "pthread.h"

typedef struct {
  pthread_t pthreadId;
} thread;

int thread_create(thread* thread, void*(*entry_function)(void* arg), void* arg);

int thread_join(thread* thread, void** retval);

#endif
