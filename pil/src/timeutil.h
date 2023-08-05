#ifndef STROBE_PIL_TIMEUTIL_H
#define STROBE_PIL_TIMEUTIL_H

#include <time.h>

typedef struct{
  struct timespec start;
  struct timespec end;
} timer;

unsigned long long time_in_nanos();

void timer_start(timer* timer);

void timer_stop(timer* timer);

unsigned long long timer_get_nanos(timer* timer);

unsigned long long timer_get_millis(timer* timer);

float timer_get_millisf(timer* timer);

#endif
