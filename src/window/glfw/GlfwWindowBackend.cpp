#include "window/glfw/GlfwWindowBackend.hpp"

#include <map>
#include <stdexcept>

#include "window/glfw/GlfwKeyboard.hpp"
#include "window/glfw/GlfwMouse.hpp"

namespace sge::glfw {

static std::map<GLFWwindow *, WindowBackend *> instanceMapping;

void GlfwWindowBackend::initLib() {
  static bool successful = glfwInit();
  if (!successful) {
    throw std::runtime_error("could not initalize glfw3");
  }
}

static void mousePosCallback(GLFWwindow *window, double xpos, double ypos) {
  WindowBackend *instance = instanceMapping[window];
  static_cast<GlfwMouse &>(instance->getMouse())
      .setPos(xpos / instance->getWidth(),
              1.0 - (ypos / instance->getHeight()));
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  WindowBackend *instance = instanceMapping[window];
  static_cast<GlfwMouse &>(instance->getMouse()).setKeyState(button, action);
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mod) {
  WindowBackend *instance = instanceMapping[window];
  static_cast<GlfwKeyboard &>(instance->getKeyboard()).setKeyState(key, action);
}

static void sizeCallback(GLFWwindow *window, int width, int height) {
  WindowBackend *instance = instanceMapping[window];
  GlfwMouse &mouse = static_cast<GlfwMouse &>(instance->getMouse());
  mouse.setPos((mouse.getX() * instance->getWidth()) / width,
               1.0 - ((1.0 - mouse.getY()) * instance->getHeight()) / height);
  mouse.setPPos(mouse.getX(), mouse.getY());
  instance->setSizeInternal(width, height, true);
}

// Window(const uint32_t width, const uint32_t height, const strb::string title,
//        const ClientApi clientApi, const WindowApi windowApi, Mouse *mouse,
//        Keyboard *keyboard);

GlfwWindowBackend::GlfwWindowBackend(
    const u32 width, const u32 height, const std::string title,
    const RendererBackendAPI renderer_backend_api)
    : WindowBackend(width, height, title, renderer_backend_api,
                    std::make_unique<GlfwMouse>(),
                    std::make_unique<GlfwKeyboard>()) {
  initLib();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  if (renderer_backend_api == VULKAN_RENDER_BACKEND) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  } else {
    throw std::runtime_error("RenderApi is not supported by a GLFW-Window");
  }
  ptr = glfwCreateWindow(getWidth(), getHeight(), title.data(), NULL, NULL);
  if (!ptr) {
    glfwTerminate();
    throw std::runtime_error("could not initalize GLFWwindow");
  }
  instanceMapping[ptr] = this;
  glfwSetCursorPosCallback(ptr, mousePosCallback);
  glfwSetMouseButtonCallback(ptr, mouseButtonCallback);
  glfwSetKeyCallback(ptr, keyCallback);
  glfwSetWindowSizeCallback(ptr, sizeCallback);
}

boolean GlfwWindowBackend::shouldClose() { return glfwWindowShouldClose(ptr); }

void GlfwWindowBackend::swapBuffers() { glfwSwapBuffers(ptr); }
void GlfwWindowBackend::pollEvents() {
  static_cast<GlfwMouse &>(getMouse()).rotate();
  // static_cast<Keyboard &>(getKeyboard()).rotate();
  glfwPollEvents();
}

void GlfwWindowBackend::close() { glfwSetWindowShouldClose(ptr, GLFW_FALSE); }
void GlfwWindowBackend::destroy() {
  glfwDestroyWindow(ptr);
  glfwTerminate();
}

void GlfwWindowBackend::setWidth(u32 width) {
  if (this->getWidth() == width) return;
  glfwSetWindowSize(ptr, width, getHeight());
  this->setWidthInternal(width);
}

void GlfwWindowBackend::setHeight(u32 height) {
  if (this->getHeight() == height) return;
  glfwSetWindowSize(ptr, getWidth(), height);
  this->setHeightInternal(height);
}

void GlfwWindowBackend::setSize(u32 width, u32 height) {
  if (this->getWidth() == width && this->getHeight() == height) return;
  glfwSetWindowSize(ptr, width, height);
  this->setSizeInternal(width, height, true);
}

}  // namespace sge::glfw
