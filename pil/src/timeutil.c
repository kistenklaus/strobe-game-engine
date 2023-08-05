#include "timeutil.h"

#include <bits/time.h>

unsigned long long time_in_nanos(){
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return time.tv_nsec + time.tv_nsec * 10000000000;
}

void timer_start(timer* timer){
  clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

void timer_stop(timer* timer){
  clock_gettime(CLOCK_MONOTONIC, &timer->end);
}


unsigned long long timer_get_nanos(timer* timer){
return (timer->end.tv_nsec - timer->start.tv_nsec) + (timer->end.tv_sec - timer->start.tv_sec) * 1000000000;
}

unsigned long long timer_get_millis(timer* timer){
  return timer_get_nanos(timer) / 1000;
}

float timer_get_millisf(timer* timer){
  return timer_get_nanos(timer) / 1000.0f;
}
