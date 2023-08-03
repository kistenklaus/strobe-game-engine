#ifndef STROBE_RENDERER_FRAME_GUARD_H
#define STROBE_RENDERER_FRAME_GUARD_H

#include "semaphore.h"

typedef struct {
  sem_t semFramesReady;
  sem_t semFramesEmpty;

  sem_t semSubmitBarrier;
  sem_t semCompleteBarrier;

  unsigned int submitFrameIdx;
  unsigned int completeFrameIdx;
  unsigned int frameCount;
} frame_guard_t;

void frame_guard_init(frame_guard_t *fg, unsigned int frameCount);

void frame_guard_acquire_submit_frame(frame_guard_t *fg, unsigned int* submitFrame);

void frame_guard_release_submit_frame(frame_guard_t *fg);

void frame_guard_acquire_complete_frame(frame_guard_t *fg, unsigned int* completeFrame);

void frame_guard_release_complete_frame(frame_guard_t *fg);

int frame_guard_destroy(frame_guard_t *fg);

#endif
