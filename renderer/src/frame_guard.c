#include "frame_guard.h"
#include <stdlib.h>

void frame_guard_init(frame_guard_t *fg, unsigned int frameCount) {
  sem_init(&fg->semFramesReady, 0, 0);
  sem_init(&fg->semFramesEmpty, 0, frameCount);
  fg->submitFrameIdx = 0;
  fg->completeFrameIdx = 0;
  fg->frameCount = frameCount;
}

void frame_guard_acquire_submit_frame(frame_guard_t *fg,
                                      unsigned int *submitFrame) {
  sem_wait(&fg->semFramesEmpty);
  *submitFrame = fg->submitFrameIdx;
  fg->submitFrameIdx = (fg->submitFrameIdx + 1) % fg->frameCount;
}

void frame_guard_release_submit_frame(frame_guard_t *fg) {
  sem_post(&fg->semFramesReady);
}

void frame_guard_acquire_complete_frame(frame_guard_t *fg,
                                        unsigned int *completeFrame) {
  sem_wait(&fg->semFramesReady);
  *completeFrame = fg->completeFrameIdx;
  fg->completeFrameIdx = (fg->completeFrameIdx + 1) % fg->frameCount;
}

void frame_guard_release_complete_frame(frame_guard_t *fg) {
  sem_post(&fg->semFramesEmpty);
}

int frame_guard_destroy(frame_guard_t *fg) {
  int erno1 = sem_destroy(&fg->semFramesReady);
  int erno2 = sem_destroy(&fg->semFramesEmpty);
  return erno1 | erno2;
}
