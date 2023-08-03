#ifndef STROBE_RENDERER_FRAME_BARRIER_H
#define STROBE_RENDERER_FRAME_BARRIER_H

#include "semaphore.h"

typedef struct  {
  sem_t semSubmitBarrier;
  sem_t semCompleteBarrier;
} frame_barrier_t;

void frame_barrier_init(frame_barrier_t* fb);

void frame_barrier_submit_wait(frame_barrier_t *fb);

void frame_barrier_complete_wait(frame_barrier_t *fb);

void frame_barrier_destroy(frame_barrier_t* fb);

#endif
