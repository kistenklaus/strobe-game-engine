#include "frame_barrier.h"

void frame_barrier_init(frame_barrier_t *fb) {
  sem_init(&fb->semSubmitBarrier, 0, 0);
  sem_init(&fb->semCompleteBarrier, 0, 0);
}

void frame_barrier_submit_wait(frame_barrier_t *fb) {
  sem_wait(&fb->semCompleteBarrier);
  sem_post(&fb->semSubmitBarrier);
}

void frame_barrier_complete_wait(frame_barrier_t *fb) {
  sem_post(&fb->semCompleteBarrier);
  sem_wait(&fb->semSubmitBarrier);
}

void frame_barrier_destroy(frame_barrier_t *fb) {
  sem_destroy(&fb->semSubmitBarrier);
  sem_destroy(&fb->semCompleteBarrier);
}
