#include "thread.h"

int thread_create(thread_t *thread, void *(*entry_function)(void *arg),
                  void *arg) {
  return pthread_create(&(thread->pthreadId), NULL, entry_function, arg);
}

int thread_join(thread_t* thread, void** retval){
  return pthread_join(thread->pthreadId, retval);
}
