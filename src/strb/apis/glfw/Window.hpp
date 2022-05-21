#pragma once
#include "strb/apis/glfw/Keyboard.hpp"
#include "strb/apis/glfw/Mouse.hpp"
#include "strb/apis/glfw/glfw.lib.hpp"
#include "strb/core/window/Window.hpp"
#include "strb/stl.hpp"

namespace strb::glfw {

class Window : public strb::window::Window {
 public:
 private:
  GLFWwindow *ptr;

 public:
  Window(const uint32_t windowWidth, const uint32_t windowHeight,
         const strb::string title, const strb::window::ClientApi clientApi);
  ~Window() override = default;
  strb::boolean shouldClose() override;
  void swapBuffers() override;
  void pollEvents() override;
  void close() override;
  void destroy() override;
  void setWidth(uint32_t width) override;
  void setHeight(uint32_t height) override;
  void setSize(uint32_t width, uint32_t height) override;
  inline const GLFWwindow *pointer() const { return ptr; }

 private:
  static void initLib();
};

}  // namespace strb::glfw
