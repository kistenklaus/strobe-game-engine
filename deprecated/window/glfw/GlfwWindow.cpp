#include "./GlfwWindow.hpp"

#include <GLFW/glfw3.h>

#include <cstring>
#include <mutex>
#include <print>
#include <stdexcept>
#include <utility>
#include <variant>

#include "window/ClientApi.hpp"
#include "window/WindowControlBlock.hpp"

namespace strobe::window {

static thread_local std::mutex s_glfwUseCountMutex;
static thread_local std::size_t s_glfwUseCount;

static void window_size_callback(GLFWwindow* window, int width, int height) {
  details::WindowControlBlock* controlBlock =
      reinterpret_cast<details::WindowControlBlock*>(
          glfwGetWindowUserPointer(window));
  std::lock_guard<std::mutex> lck{controlBlock->mutex};
  controlBlock->width = width;
  controlBlock->height = height;
}

static void framebuffer_size_callback(GLFWwindow* window, int framebufferWidth,
                                      int framebufferHeight) {
  details::WindowControlBlock* controlBlock =
      reinterpret_cast<details::WindowControlBlock*>(
          glfwGetWindowUserPointer(window));
  std::lock_guard<std::mutex> lck{controlBlock->mutex};
  controlBlock->framebufferWidth = framebufferWidth;
  controlBlock->framebufferHeight = framebufferHeight;
}

GlfwWindow::GlfwWindow(
    ClientApi api,
    const std::shared_ptr<details::WindowControlBlock>& controlBlock) {
  {
    std::lock_guard<std::mutex> lck{s_glfwUseCountMutex};
    if (s_glfwUseCount == 0) {
      if (!glfwInit()) {
        throw std::runtime_error("Failed to initalize GLFW");
      }
    }
    s_glfwUseCount++;
  }

  std::lock_guard<std::mutex> lck{controlBlock->mutex};

  if (std::holds_alternative<ClientApiVulkan>(*api)) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  } else {
    throw std::runtime_error("Unsupported Client API");
  }
  glfwWindowHint(GLFW_RESIZABLE,
                 controlBlock->resizable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

  GLFWwindow* ptr =
      glfwCreateWindow(controlBlock->width, controlBlock->height,
                       controlBlock->title.data(), nullptr, nullptr);

  if (!ptr) {
    glfwTerminate();
    std::runtime_error("Failed to create GLFW window");
  }
  glfwSwapInterval(controlBlock->vsync ? 1 : 0);

  glfwSetWindowUserPointer(ptr, controlBlock.get());
  glfwSetWindowSizeCallback(ptr, window_size_callback);

  int framebufferWidth, framebufferHeight;
  glfwGetFramebufferSize(ptr, &framebufferWidth, &framebufferHeight);
  controlBlock->framebufferWidth = framebufferWidth;
  controlBlock->framebufferHeight = framebufferHeight;
  glfwSetFramebufferSizeCallback(ptr, framebuffer_size_callback);

  m_ptr = ptr;

  glfwSetWindowAttrib(ptr, GLFW_VISIBLE,
                      controlBlock->visible ? GLFW_TRUE : GLFW_FALSE);
}

GlfwWindow::~GlfwWindow() {
  glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(m_ptr));
  std::lock_guard<std::mutex> lck{s_glfwUseCountMutex};
  s_glfwUseCount--;
  if (s_glfwUseCount == 0) {
    glfwTerminate();
  }
  m_ptr = nullptr;
}

bool GlfwWindow::shouldClose() {
  return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_ptr));
}

void GlfwWindow::pollEvents() { glfwPollEvents(); }

void GlfwWindow::close() {
  glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_ptr), 1);
}

void GlfwWindow::setWidth(unsigned int width) { setSize(width, getHeight()); }

void GlfwWindow::setHeight(unsigned int height) { setSize(getWidth(), height); }

void GlfwWindow::setSize(unsigned int width, unsigned int height) {
  glfwSetWindowSize(reinterpret_cast<GLFWwindow*>(m_ptr), width, height);
}

unsigned int GlfwWindow::getWidth() { return getSize().first; }

unsigned int GlfwWindow::getHeight() { return getSize().second; }

std::pair<unsigned int, unsigned int> GlfwWindow::getSize() {
  int width;
  int height;
  glfwGetWindowSize(reinterpret_cast<GLFWwindow*>(m_ptr), &width, &height);
  return std::make_pair(width, height);
}

void GlfwWindow::setTitle(std::string_view title) {
  std::string cstr(title);
  glfwSetWindowTitle(reinterpret_cast<GLFWwindow*>(m_ptr), cstr.c_str());
};

std::string_view GlfwWindow::getTitle() {
  const char* title = glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(m_ptr));
  std::size_t size = std::strlen(title);
  return std::string_view{title, size};
}
bool GlfwWindow::isResizable() { return false; };
void GlfwWindow::setResizable(bool resizable) {};

vk::SurfaceKHR GlfwWindow::createSurface(vk::Instance instance) {
  VkSurfaceKHR surface;
  VkResult result = glfwCreateWindowSurface(
      instance, reinterpret_cast<GLFWwindow*>(m_ptr), nullptr, &surface);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create GLFW window surface for vulkan");
  }
  return surface;
};

std::span<const char* const> GlfwWindow::getRequiredVkInstanceExtensions() {
  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  return std::span(extensions, count);
}
}  // namespace strobe::window
