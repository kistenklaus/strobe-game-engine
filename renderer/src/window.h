#ifndef STROBE_RENDERER_WINDOW_H
#define STROBE_RENDERER_WINDOW_H

typedef struct window_t{
  unsigned int width;
  unsigned int height;
  char* title;
  int shouldClose;
  int (*init)(struct window_t* window);
  void (*poll_events)(struct window_t* window);
  void (*swap_buffers)(struct window_t* window);
  void (*destroy)(struct window_t* window);
  void* apiMemory;
} window_t;

#endif
