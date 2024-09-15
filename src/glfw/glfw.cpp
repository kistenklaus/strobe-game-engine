
#include "./glfw.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

GLFWwindow* strobe::glfw::g_window = nullptr;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr std::string TITLE = "testing";


void strobe::glfw::init() {
  if (glfwInit() == GLFW_FALSE) {
    throw std::runtime_error("Failed to initalize GLFW");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  g_window = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), nullptr, nullptr);
  if (g_window == nullptr) {
    throw std::runtime_error("Failed to create GLFW window");
  }
}

bool strobe::glfw::shouldClose() {
  return glfwWindowShouldClose(g_window);
}

void strobe::glfw::pollEvents() {
  glfwPollEvents();
}

void strobe::glfw::destroy() {
  glfwDestroyWindow(g_window);
}
