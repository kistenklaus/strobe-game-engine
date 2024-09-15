#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace strobe::glfw {

extern GLFWwindow* g_window;

void init();

bool shouldClose();

void pollEvents();

void destroy();

}
