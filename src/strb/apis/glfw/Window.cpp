#include "strb/apis/glfw/Window.hpp"

namespace strb::glfw {

static strb::map<GLFWwindow *, Window *> instanceMapping;

void Window::initLib() {
  static bool successful = glfwInit();
  if (!successful) {
    throw strb::runtime_exception("could not initalize glfw3");
  }
}

static void mousePosCallback(GLFWwindow *window, double xpos, double ypos) {
  Window *instance = instanceMapping[window];
  static_cast<Mouse &>(instance->getMouse())
      .setPos(xpos / instance->getWidth(),
              1.0 - (ypos / instance->getHeight()));
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  Window *instance = instanceMapping[window];
  static_cast<Mouse &>(instance->getMouse()).setKeyState(button, action);
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mod) {
  Window *instance = instanceMapping[window];
  static_cast<Keyboard &>(instance->getKeyboard()).setKeyState(key, action);
}

static void sizeCallback(GLFWwindow *window, int width, int height) {
  Window *instance = instanceMapping[window];
  Mouse &mouse = static_cast<Mouse &>(instance->getMouse());
  mouse.setPos((mouse.getX() * instance->getWidth()) / width,
               1.0 - ((1.0 - mouse.getY()) * instance->getHeight()) / height);
  mouse.setPPos(mouse.getX(), mouse.getY());
  instance->setSizeInternal(width, height, true);
}

// Window(const uint32_t width, const uint32_t height, const strb::string title,
//        const ClientApi clientApi, const WindowApi windowApi, Mouse *mouse,
//        Keyboard *keyboard);

Window::Window(const uint32_t width, const uint32_t height,
               const strb::string title,
               const strb::window::ClientApi clientApi)
    : strb::window::Window(width, height, title, clientApi,
                           strb::window::WINDOW_API_GLFW, new Mouse(),
                           new Keyboard()) {
  initLib();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  if (clientApi == strb::window::RENDER_API_VULKAN) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  } else {
    throw strb::runtime_exception(
        "RenderApi is not supported by a GLFW-Window");
  }
  ptr = glfwCreateWindow(getWidth(), getHeight(), title.data(), NULL, NULL);
  if (!ptr) {
    glfwTerminate();
    throw strb::runtime_exception("could not initalize GLFWwindow");
  }
  instanceMapping[ptr] = this;
  glfwSetCursorPosCallback(ptr, mousePosCallback);
  glfwSetMouseButtonCallback(ptr, mouseButtonCallback);
  glfwSetKeyCallback(ptr, keyCallback);
  glfwSetWindowSizeCallback(ptr, sizeCallback);
}

strb::boolean Window::shouldClose() { return glfwWindowShouldClose(ptr); }

void Window::swapBuffers() { glfwSwapBuffers(ptr); }
void Window::pollEvents() {
  static_cast<Mouse &>(getMouse()).rotate();
  // static_cast<Keyboard &>(getKeyboard()).rotate();
  glfwPollEvents();
}

void Window::close() { glfwSetWindowShouldClose(ptr, GLFW_FALSE); }
void Window::destroy() {
  glfwDestroyWindow(ptr);
  glfwTerminate();
}

void Window::setWidth(uint32_t width) {
  if (this->getWidth() == width) return;
  glfwSetWindowSize(ptr, width, getHeight());
  this->setWidthInternal(width);
}

void Window::setHeight(uint32_t height) {
  if (this->getHeight() == height) return;
  glfwSetWindowSize(ptr, getWidth(), height);
  this->setHeightInternal(height);
}

void Window::setSize(uint32_t width, uint32_t height) {
  if (this->getWidth() == width && this->getHeight() == height) return;
  glfwSetWindowSize(ptr, width, height);
  this->setSizeInternal(width, height, true);
}

}  // namespace strb::glfw
