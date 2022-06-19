#pragma once
#include <string>

#include "types/inttypes.hpp"
#include "window/WindowBackend.hpp"
#include "window/glfw/glfw.lib.hpp"

namespace sge::glfw {

class GlfwWindowBackend : public WindowBackend {
 public:
 private:
  GLFWwindow *ptr;

 public:
  GlfwWindowBackend(const u32 windowWidth, const u32 windowHeight,
                    const std::string title,
                    const RendererBackendAPI render_backend_api);
  ~GlfwWindowBackend() override = default;
  boolean shouldClose() override;
  void swapBuffers() override;
  void pollEvents() override;
  void close() override;
  void destroy() override;
  void setWidth(u32 width) override;
  void setHeight(u32 height) override;
  void setSize(u32 width, u32 height) override;
  const GLFWwindow *pointer() const { return ptr; }

 private:
  static void initLib();
};

}  // namespace sge::glfw
