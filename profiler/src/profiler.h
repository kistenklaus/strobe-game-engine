#ifndef STROBE_PROFILER_PROFILER_H

#define STROBE_PROFILER_PROFILER_H

#include "timeutil.h"
#include "containers.h"

typedef struct {
  unsigned int historySize;
} frame_profiler_create_info;

typedef struct {
  char* name;
  timer timer;
} frame_profiler_block;

typedef struct {
  unsigned int historySize;

  unsigned int frameCount;
  unsigned int currentFrame;

  darray* frameBlocks;
  darray* frameBlockStack;
} frame_profiler;


void frame_profiler_init(frame_profiler *profiler,
                         frame_profiler_create_info *createInfo);

void frame_profiler_begin_frame(frame_profiler* profiler);

void frame_profiler_push_block(frame_profiler* profiler, char* blockName);

void frame_profiler_pop_block(frame_profiler* profiler);

void frame_profiler_end_frame(frame_profiler* profiler);

void frame_profiler_destroy(frame_profiler *profiler);

unsigned long long  frame_profiler_average_frametime(frame_profiler* profiler);

unsigned long long frame_profiler_get_last_frametime(frame_profiler* profiler);

unsigned long long frame_profiler_max_frametime(frame_profiler* profiler);

unsigned long long frame_profiler_min_frametime(frame_profiler* profiler);




#endif
