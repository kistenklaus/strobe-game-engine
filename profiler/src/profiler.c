#include "profiler.h"
#include <stdio.h>
#include <stdlib.h>

void frame_profiler_init(frame_profiler *profiler,
                         frame_profiler_create_info *createInfo) {
  profiler->historySize = createInfo->historySize;

  profiler->frameCount = 0;
  profiler->currentFrame = 0;

  profiler->frameBlocks = calloc(profiler->historySize, sizeof(darray));
  profiler->frameBlockStack = calloc(profiler->historySize, sizeof(darray));
  for (unsigned int i = 0; i < profiler->historySize; i++) {
    darray_create(profiler->frameBlocks + i, sizeof(frame_profiler_block), 16);
    darray_create(profiler->frameBlockStack + i, sizeof(unsigned int), 16);
  }
}

void frame_profiler_destroy(frame_profiler *profiler) {
  for (unsigned int i = 0; i < profiler->historySize; i++) {
    darray_destroy(profiler->frameBlocks + i);
    darray_destroy(profiler->frameBlockStack + i);
  }
  free(profiler->frameBlocks);
  free(profiler->frameBlockStack);
}

void frame_profiler_begin_frame(frame_profiler *profiler) {

  darray *stack = profiler->frameBlockStack + profiler->currentFrame;
  darray_clear(stack);
  *(unsigned int *)darray_emplace_back(stack) = 0;

  darray *blocks = profiler->frameBlocks + profiler->currentFrame;
  darray_clear(blocks);
  frame_profiler_block *block = darray_emplace_back(blocks);
  block->name = NULL;
  timer_start(&block->timer);
}

void frame_profiler_push_block(frame_profiler *profiler, char *blockName) {
  darray *blocks = profiler->frameBlocks + profiler->currentFrame;
  darray *stack = profiler->frameBlockStack + profiler->currentFrame;

  *(unsigned int *)darray_emplace_back(stack) = darray_size(blocks);

  frame_profiler_block *block = darray_emplace_back(blocks);
  block->name = blockName;
  timer_start(&block->timer);
}

void frame_profiler_pop_block(frame_profiler *profiler) {
  darray *blocks = profiler->frameBlocks + profiler->currentFrame;
  darray *stack = profiler->frameBlockStack + profiler->currentFrame;

  unsigned int blockId = *(unsigned int *)darray_get_back(stack);
  darray_pop_back(stack);
  timer_stop(&((frame_profiler_block *)darray_get(blocks, blockId))->timer);
}

void frame_profiler_end_frame(frame_profiler *profiler) {
  frame_profiler_pop_block(profiler);
  frame_profiler_block *block =
      darray_get(profiler->frameBlocks + profiler->currentFrame, 0);
  unsigned long long ft = timer_get_nanos(&block->timer);
  profiler->currentFrame++;
  if (profiler->currentFrame >= profiler->historySize) {
    profiler->currentFrame = 0;
  }
  if (profiler->frameCount < profiler->historySize) {
    profiler->frameCount++;
  }

  frame_profiler_block *block0 =
      darray_get(profiler->frameBlocks + profiler->currentFrame, 0);
}

unsigned long long frame_profiler_average_frametime(frame_profiler *profiler) {
  unsigned long long frametime = 0;

  frame_profiler_block *block0 = darray_get(profiler->frameBlocks + 0, 0);

  for (unsigned int i = 0; i < profiler->frameCount; i++) {
    frametime += timer_get_nanos(
        &((frame_profiler_block *)darray_get(profiler->frameBlocks + i, 0))
             ->timer);
  }

  return frametime / profiler->frameCount;
}

unsigned long long frame_profiler_get_last_frametime(frame_profiler* profiler){
  if(profiler->currentFrame != 0){
    return timer_get_nanos(
        &((frame_profiler_block *)darray_get(profiler->frameBlocks + profiler->currentFrame - 1, 0))
             ->timer);
  }else{
    return timer_get_nanos(
        &((frame_profiler_block *)darray_get(profiler->frameBlocks + profiler->historySize - 1, 0))
             ->timer);
  }
}

unsigned long long frame_profiler_max_frametime(frame_profiler *profiler) {
  unsigned long long max = 0;
  for (unsigned int i = 0; i < profiler->frameCount; i++) {
    unsigned long long frametime = timer_get_nanos(
        &((frame_profiler_block *)darray_get(profiler->frameBlocks + i, 0))
             ->timer);
    if (frametime > max) {
      max = frametime;
    }
  }
  return max;
}

unsigned long long frame_profiler_min_frametime(frame_profiler *profiler) {
  unsigned long long min = -1;
  for (unsigned int i = 0; i < profiler->frameCount; i++) {
    unsigned long long frametime = timer_get_nanos(
        &((frame_profiler_block *)darray_get(profiler->frameBlocks + i, 0))
             ->timer);
    if (frametime < min) {
      min = frametime;
    }
  }
  return min;
}
