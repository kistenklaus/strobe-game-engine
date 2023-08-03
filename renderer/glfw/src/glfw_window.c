#include "glfw_window.h"
#include "GLFW/glfw3.h"
#include "stdio.h"
#include <stdlib.h>


/*
 *
  void (*init)(struct window_t* window);
  void (*poll_events)(struct window_t* window);
  void (*swap_buffers)(struct window_t* window);
  void (*destroy)(struct window_t* window);
  void* apiMemory;
 * */

static GLFWwindow* glfwWindow;

static int glfwInitalized = 0;

static void glfw_window_init(window_t* window) {
  if(!glfwInitalized){
    glfwInitalized = glfwInit();
    if(!glfwInitalized){
      printf("Failed to initalize glfw\n");
      exit(-1);
    }
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_WAYLAND_APP_ID, 0);

  glfwWindow = glfwCreateWindow(window->width, window->height, window->title, NULL, NULL);
  if(glfwWindow == NULL) {
    printf("Failed to create glfw window!\n");
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(glfwWindow);
}

static void glfw_window_poll_events(window_t* window) {
  glfwPollEvents();
  window->shouldClose = glfwWindowShouldClose(glfwWindow);
}

static void glfw_window_swap_buffers(window_t* window) {
  glfwSwapBuffers(glfwWindow);
}

static void glfw_window_destroy(window_t* window) {
  glfwDestroyWindow(glfwWindow);
}


void create_glfw_window(window_t* window, unsigned int width, unsigned int height,
    char* title){
  window->width = width;
  window->height = height;
  window->title = title;
  window->shouldClose = 0;
  window->init = glfw_window_init;
  window->poll_events = glfw_window_poll_events;
  window->swap_buffers = glfw_window_swap_buffers;
  window->destroy = glfw_window_destroy;
} 
